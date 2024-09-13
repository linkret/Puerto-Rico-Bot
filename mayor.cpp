#include "mayor.h"
#include "game.h"

#include <iostream>

void MayorAction::perform(GameState& g, const Action& action) const {
    auto& player = g.player_state[g.current_player_idx];

    int took_from_supply = 0;
    if (player.idx == g.current_round_player_idx) {
        std::fill(g.colonists_for_player, g.colonists_for_player + 5, 0);
        g.colonists_for_player[player.idx] += g.colonist_supply > 0; // TODO: Mayor bonus could be refused, but we always accept it for now

        for (int i = 0; i < g.colonist_ship; i++) {
            g.colonists_for_player[(player.idx + i) % g.player_count] += 1;
        }

        if (g.colonist_supply > 0) {
            if (g.verbose)
                std::cout << "Player " << player.idx << " took 1 extra Colonist from the supply" << std::endl;
            
            took_from_supply++;
        }
    }

    int took_from_ship = g.colonists_for_player[player.idx] - took_from_supply;
    g.colonist_ship -= took_from_ship;
    g.colonist_supply -= took_from_supply;
    g.colonists_for_player[player.idx] = 0;

    auto dist_plant = action.mayor_allocation.distribution;
    auto dist_build = action.mayor_allocation.distribution;
    int extras = action.mayor_allocation.extra_colonists;

    std::vector<PlantationState> new_plantations;
    std::vector<BuildingState> new_buildings;

    for (auto plantation : player.plantations) {
        int pidx = static_cast<int>(plantation.plantation);

        if (dist_plant.w[pidx] > 0) {
            plantation.colonists = 1;
            dist_plant.w[pidx]--;
        }
        else if (extras > 0) {
            // Note: this can leave an empty Quarry even when there's extra colonists if the MayorAction was not optimally chosen. We could sort plantations.
            plantation.colonists = 1; // TODO: always places extras on plantations, even if sometimes (slightly) not optimal
            extras--;
        } 
        else {
            plantation.colonists = 0;
        }
        
        new_plantations.push_back(plantation);
    }

    const auto& buildings = action.mayor_allocation.buildings;

    for (auto building : player.buildings) {
        if (building.building.good_produced != Good::NONE) {
            int gidx = static_cast<int>(building.building.good_produced);

            if (dist_build.w[gidx] > 0) {
                int num_cols = std::min(dist_build.w[gidx], building.building.capacity);
                building.colonists = num_cols;
                dist_build.w[gidx] -= num_cols;
            }
            else {
                building.colonists = 0;
            }
        } else if (std::find(buildings.begin(), buildings.end(), building.building.type) != buildings.end()) {
            building.colonists = 1;
        } else {
            building.colonists = 0;
        }

        if (extras > 0) {
            int num_cols = std::min(extras, building.building.capacity - building.colonists);
            building.colonists += num_cols;
            extras -= num_cols;
        }

        new_buildings.push_back(building);
    }

    player.plantations = new_plantations;
    player.buildings = new_buildings;
    player.extra_colonists = extras;

    if (g.verbose) {
        std::cout << "Player " << player.idx << " distributed Colonists:" << std::endl;

        // TODO: seperate into player.print_colonists()

        std::cout << "Buildings: ";
        for (const auto& building : player.buildings) {
            std::cout << building_name(building.building) << " " << building.colonists << "/" << building.building.capacity << ", ";
        }
        std::cout << std::endl;

        std::cout << "Plantations: ";
        for (const auto& plantation : player.plantations) {
            std::cout << plantation_name(plantation) << " " << plantation.colonists << "/1, ";
        }
        std::cout << std::endl;

        if (player.extra_colonists > 0)
            std::cout << "Extra Colonists: " << player.extra_colonists << std::endl;
    }
    
    g.next_player();
}

std::vector<Action> MayorAction::get_legal_actions(const GameState& g, bool is_mayor) const {
    // Brute-forcing all possible colonist allocations here would not be feasible.
    // (20 colonist slots with 12 total colonists would result in over 100k possibilities.)
    // Instead, we will generate up to 100 distributions of which goods to produce (also counting Quarries as a Good).
    // All goods are produced in matching plantation-factory pairs:
    // For example, it wouldn't make much sense to allocate 3 workers in an indigo factory but none in indigo plantations.
    // It also wouldn't make sense to put 2 workers in a coffee factory but only 1 in a coffee plantation, either.
    // So we will generate all possible combinations of produced-goods (for example, 1 Corn 2 Indigo 1 Coffee 1 Quarry)
    // Then for every combination we generate up to 10 possible allocations of remaining workers to non-production buildings.
    // In total, the number of generated legal Actions will not exceed 200. (Can be changed to further prune the search tree.)

    // TODO: implement an alternate, simple strategy that only allocates new Colonists without removing any previous ones

    std::vector<Action> actions;

    auto& player = g.player_state[g.current_player_idx];

    int colonists_for_player[5];
    if (is_mayor) {
        // TODO: replicate for g.colonists_for_player
        std::fill(colonists_for_player, colonists_for_player + 5, 0);

        colonists_for_player[player.idx] += (is_mayor && g.colonist_supply > 0); // TODO: Mayor bonus could be refused, but we always accept it for now

        for (int i = 0; i < g.colonist_ship; i++) {
            colonists_for_player[(player.idx + i) % g.player_count] += 1;
        }
    } else {
        std::copy(g.colonists_for_player, g.colonists_for_player + 5, colonists_for_player);
    }

    int total_colonists = player.get_total_colonists() + colonists_for_player[player.idx];
    
    std::vector<BuildingType> nonprod_buildings;
    for (const auto& building : player.buildings) {
        if (building.building.good_produced == Good::NONE)
            nonprod_buildings.push_back(building.building.type);
    }

    auto max_goods = player.get_producing_goods(true); // theoretical maximum goods produced
    int querries = player.get_querry_count(true);
    int max_employed = max_goods[0].count + 2 * max_goods[1].count + 2 * max_goods[2].count + 2 * max_goods[3].count + 2 * max_goods[4].count + querries;

    int DISTRIBUTION_LIMIT = 100; // arbitrary limit
    std::vector<ProductionDistribution> distributions;

    // TODO: try to write this in a more readable way
    for (int corn = max_goods[0].count; corn >= 0 ; corn--) {
        int c_col = total_colonists - corn;
        if (c_col < 0)
            continue;

        for (int indigo = max_goods[1].count; indigo >= 0; indigo--) {
            int i_col = c_col - 2 * indigo;
            if (i_col < 0)
                continue;

            for (int sugar = max_goods[2].count; sugar >= 0 ; sugar--) {
                int s_col = i_col - 2 * sugar;
                if (s_col < 0)
                    continue;

                for (int tobacco = max_goods[3].count; tobacco >= 0; tobacco--) {
                    int t_col = s_col - 2 * tobacco;
                    if (t_col < 0)
                        continue;

                    for (int coffee = max_goods[4].count; coffee >= 0; coffee--) {
                        int co_col = t_col - 2 * coffee;
                        if (co_col < 0)
                            continue;

                        for (int querry = querries; querry >= 0 ; querry--) {
                            int q_col = co_col - querry;
                            if (q_col < 0)
                                continue;

                            int employed = total_colonists - q_col;

                            // do not allow under-employment - always dominated by a previous distribution
                            if (employed < max_employed && q_col > nonprod_buildings.size() + 1)
                                continue;

                            distributions.push_back({corn, indigo, sugar, tobacco, coffee, querry});

                            if (distributions.size() > DISTRIBUTION_LIMIT)
                                goto out;
                        }
                    }
                }
            }
        }
    }

    out:;

    if (distributions.empty()) {
        throw std::runtime_error("No legal Mayor colonist distributions generated");
        // shouldn't happen, recovery method: //distributions.push_back({0, 0, 0, 0, 0, 0});
    }

    int max_allocs_per_dist = std::min(20, 2 * DISTRIBUTION_LIMIT / static_cast<int>(distributions.size())); // arbitrary limit

    auto rng = g.rng; // g.rng is const, therefore unusable for std::shuffle

    for (const auto& dist : distributions) {
        int total_plantation = dist.corn() + 2 * dist.indigo() + 2 * dist.sugar() + 2 * dist.tobacco() + 2 * dist.coffee() + dist.querry();
        int total_building = total_colonists - total_plantation;
        int total_extra = std::max(0, total_building - int(nonprod_buildings.size()));

        for (int i = 0; i < max_allocs_per_dist; i++) {
            auto buildings_copy = nonprod_buildings;
            std::shuffle(buildings_copy.begin(), buildings_copy.end(), rng); // randomly choose remaining buildings
            if (buildings_copy.size() > total_building)
                buildings_copy.resize(total_building);
            actions.emplace_back(MayorAllocation(dist, buildings_copy, total_extra));

            if (nonprod_buildings.size() <= total_building || total_building == 0) // result would always be the same, since we have more than enough people
                break;
        }
    }

    // TODO: could remove duplicates by sorting contents of each MayorAllocation. Not very important

    return actions;
}