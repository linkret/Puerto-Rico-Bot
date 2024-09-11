#ifndef GAME_H
#define GAME_H

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <random>

enum class Good {
    CORN = 0,
    INDIGO = 1,
    SUGAR = 2,
    TOBACCO = 3,
    COFFEE = 4,
    NONE
};

const std::string GoodNames[] = {
    "Corn",
    "Indigo",
    "Sugar",
    "Tobacco",
    "Coffee",
    "None"
};

enum class Plantation {
    CORN = 0,
    INDIGO = 1,
    SUGAR = 2,
    TOBACCO = 3,
    COFFEE = 4,
    QUARRY = 5,
    NONE
};

const std::string PlantationNames[] = {
    "Corn",
    "Indigo",
    "Sugar",
    "Tobacco",
    "Coffee",
    "Quarry",
    "None"
};

struct PlantationState {
    Plantation plantation;
    int colonists;
};

enum class BuildingType {
    SMALL_INDIGO_PLANT,
    SMALL_SUGAR_MILL,
    SMALL_MARKET,
    HACIENDA,
    CONSTRUCTION_HUT,
    SMALL_WAREHOUSE,

    LARGE_INDIGO_PLANT,
    LARGE_SUGAR_MILL,
    HOSPICE,
    OFFICE,
    LARGE_MARKET,
    LARGE_WAREHOUSE,
    
    TOBACCO_STORAGE,
    COFFEE_ROASTER,
    FACTORY,
    UNIVERSITY,
    HARBOR,
    WHARF,
    
    GUILD_HALL,
    CUSTOMS_HOUSE,
    CITY_HALL,
    RESIDENCE,
    FORTRESS,

    NONE
};

std::string BuildingNames[] = {
    "Small Indigo Plant",
    "Small Sugar Mill",
    "Small Market",
    "Hacienda",
    "Construction Hut",
    "Small Warehouse",

    "Large Indigo Plant",
    "Large Sugar Mill",
    "Hospice",
    "Office",
    "Large Market",
    "Large Warehouse",

    "Tobacco Storage",
    "Coffee Roaster",
    "Factory",
    "University",
    "Harbor",
    "Wharf",

    "Guild Hall",
    "Customs House",
    "City Hall",
    "Residence",
    "Fortress",

    "None"
};

struct Building {
    BuildingType type;
    int cost;
    int max_discount;
    int victory_points;
    int capacity = 1;
    Good good_produced = Good::NONE;
};

struct BuildingState {
    Building building;
    int colonists;
};

struct BuildingSupply {
    Building building;
    int count;
};

struct GoodSupply {
    Good good;
    int count;
};

enum class PlayerRole {
    MAYOR = 0,
    CRAFTSMAN = 1,
    TRADER = 2,
    SETTLER = 3,
    BUILDER = 4,
    CAPTAIN = 5,
    PROSPECTOR = 6, // disabled in 3 player game
    PROSPECTOR_2 = 7, // disabled in 3-4 player game
    NONE
};

struct RoleState {
    PlayerRole role;
    bool taken;
    int doubloons;
};

// TODO: also use for Craftsman
struct ProductionDistribution {
    int w[6]; // corn, indigo, sugar, tobacco, coffee, querry;

    int corn() const { return w[0]; }
    int indigo() const { return w[1]; }
    int sugar() const { return w[2]; }
    int tobacco() const { return w[3]; }
    int coffee() const { return w[4]; }
    int querry() const { return w[5]; }
};

struct MayorAllocation {
    ProductionDistribution distribution;
    std::vector<BuildingType> buildings;
    int extra_colonists;

    MayorAllocation() = default;
    MayorAllocation(ProductionDistribution distribution, std::vector<BuildingType> buildings, int extra_colonists) 
        : distribution(distribution), buildings(buildings), extra_colonists(extra_colonists) {}
};

struct PlayerState {
    const int player_idx;

    int doubloons;
    int victory_points = 0;
    int extra_colonists = 0;
    int goods[5] = {0, 0, 0, 0, 0};
    std::vector<PlantationState> plantations;
    std::vector<BuildingState> buildings;
    int free_town_space = 12;

    PlayerState(int player_count, int player_idx) : player_idx(player_idx) {
        plantations.reserve(12);
        buildings.reserve(12);

        doubloons = player_count - 1; // 3/4/5 players start with 2/3/4 doubloons

        // Starting plantation:
        if (player_count == 3) {
            if (player_idx == 2)
                plantations.push_back({Plantation::CORN, 0});
            else
                plantations.push_back({Plantation::INDIGO, 0});

        } else if (player_count == 4) {
            if (player_idx >= 2)
                plantations.push_back({Plantation::CORN, 0});
            else
                plantations.push_back({Plantation::INDIGO, 0});
        } else if (player_count == 5) {
            if (player_idx >= 3)
                plantations.push_back({Plantation::CORN, 0});
            else
                plantations.push_back({Plantation::INDIGO, 0});
        }
    }

    int get_total_goods() const {
        int total_goods = 0;
        for (int i = 0; i < 5; i++) {
            total_goods += goods[i];
        }
        return total_goods;
    }

    int get_total_victory_points() const {
        int total_points = victory_points;
        
        for (const auto& building : buildings) {
            total_points += building.building.victory_points;

            if (building.colonists == 0 || building.building.cost < 10)
                continue;

            if (building.building.type == BuildingType::RESIDENCE) {
                int filled_plantations = std::max(9u, plantations.size());
                total_points += filled_plantations - 5; // 4/5/6/7 points for filled 9 or lower/10/11/12 plantation spaces
            }
            else if (building.building.type == BuildingType::CUSTOMS_HOUSE) {
                total_points += victory_points / 4;
            }
            else if (building.building.type == BuildingType::GUILD_HALL) {
                int guild_points = 0;
                for (const auto& other_building : buildings) {
                    if (other_building.building.good_produced != Good::NONE)
                        guild_points += (other_building.building.capacity == 1) ? 1 : 2; // 1/2 points for small/large good-producing buildings
                }
                total_points += guild_points;
            }
            else if (building.building.type == BuildingType::CITY_HALL) {
                int city_points = 0;
                for (const auto& other_building : buildings) {
                    if (other_building.building.good_produced == Good::NONE)
                        city_points += 1; // 1 point for each non-good-producing building
                }
                total_points += city_points;
            }
            else if (building.building.type == BuildingType::FORTRESS) {       
                total_points += get_total_colonists() / 3; // 1 point for each 3 total colonists
            }
        }

        return total_points;
    }

    int get_total_colonists() const {
        int total_colonists = extra_colonists;
        for (const auto& plantation : plantations) {
            total_colonists += plantation.colonists;
        }
        for (const auto& building : buildings) {
            total_colonists += building.colonists;
        }
        return total_colonists;
    }

    int get_querry_count() const {
        return std::count_if(plantations.begin(), plantations.end(), [](const PlantationState& plantation) {
            return plantation.plantation == Plantation::QUARRY && plantation.colonists == 1;
        });
    }

    int get_free_town_space() const {
        int free_slots = 0;
        for (const auto& building : buildings) {
            free_slots += building.building.capacity - building.colonists;
        }
        return free_slots;
    }

    std::vector<GoodSupply> get_producing_goods(bool theoretical_maximum = false) const {
        std::vector<GoodSupply> producing_goods;

        int plantation_production[5] = {0, 0, 0, 0, 0};
        int building_production[5] = {12, 0, 0, 0, 0}; // Corn doesn't need a building

        for (const auto& plantation : plantations) {
            if (plantation.plantation != Plantation::QUARRY) {
                plantation_production[static_cast<int>(plantation.plantation)] += (theoretical_maximum ? 1 : plantation.colonists);
            }
        }

        for (const auto& building : buildings) {
            if (building.building.good_produced != Good::NONE) {
                int gidx = static_cast<int>(building.building.good_produced);
                building_production[gidx] += (theoretical_maximum ? building.building.capacity : building.colonists);
            }
        }

        for (int i = 0; i < 5; i++) {
            if (theoretical_maximum || (plantation_production[i] > 0 && building_production[i] > 0)) {
                producing_goods.push_back({static_cast<Good>(i), std::min(plantation_production[i], building_production[i])});
            }
        }

        return producing_goods;
    }
};

struct Ship {
    int capacity;
    Good good;
    int good_count;
};

enum RuleSet {
    CLASSIC,
    BALANCED // more expensive Factory and Harbor, less expensive Wharf, -1 doubloon for first wheat player, ... ?
    // EXPANSION // extra buildings
};

struct Action {
    // TODO: implement all kinds of actions
    PlayerRole type;

    Building building;
    int building_cost;
    Plantation plantation;
    Good good;
    int ship_capacity;
    int sell_price;
    MayorAllocation mayor_allocation;

    Action(PlayerRole type) : type(type) {}
    Action(Building building, int cost) : type(PlayerRole::BUILDER), building(building), building_cost(cost) {}
    Action(Plantation plantation) : type(PlayerRole::SETTLER), plantation(plantation) {}
    Action(Good good) : type(PlayerRole::CRAFTSMAN), good(good) {}
    Action(Good good, int price) : type(PlayerRole::TRADER), good(good), sell_price(price) {}
    Action(MayorAllocation allocation) : type(PlayerRole::MAYOR), mayor_allocation(allocation) {}
    Action(int ship_capacity, Good good, int bonus) : type(PlayerRole::CAPTAIN), good(good), ship_capacity(ship_capacity), sell_price(bonus) {}
};

class GameState {
    // Puerto Rico board state for 3-5 players
    
    const RuleSet rule_set = RuleSet::CLASSIC;
    const bool verbose = false;
    bool game_ending = false;
    const int player_count;

    int round = 0;
    int governor_idx = 0;
    int current_round_player_idx = 0;
    int current_player_idx = 0;
    int winner = -1;
    PlayerRole current_role = PlayerRole::NONE;
    std::vector<RoleState> role_state;

    int colonist_supply;
    int colonist_ship;
    int colonists_for_player[5] = {0, 0, 0, 0, 0}; // how many each player can take durring the Mayor phase
    int victory_points_supply;
    // int doubloons_supply; // 96, probably not needed

    std::vector<GoodSupply> good_supply;

    int quarry_supply = 8;
    std::vector<Plantation> plantation_supply;
    std::vector<Plantation> plantation_offer;
    std::vector<Plantation> plantation_discard;

    std::vector<BuildingSupply> building_supply;
    
    int cant_ship_counter = 0; // for knowing when to end the Captain phase
    std::vector<Ship> ships;
    std::vector<Good> trading_house;

    std::vector<PlayerState> player_state;

public:
    GameState(int player_count, bool verbose = false) 
        : player_count(player_count), verbose(verbose)
    {
        if (player_count < 3 || player_count > 5) {
            throw std::invalid_argument("Invalid player count");
        }

        role_state = {
            {PlayerRole::MAYOR, false, 0},
            {PlayerRole::CRAFTSMAN, false, 0},
            {PlayerRole::TRADER, false, 0},
            {PlayerRole::SETTLER, false, 0},
            {PlayerRole::BUILDER, false, 0},
            {PlayerRole::CAPTAIN, false, 0}
        };

        if (player_count == 4) {
            role_state.push_back({PlayerRole::PROSPECTOR, false, 0});
        } else if (player_count == 5) {
            role_state.push_back({PlayerRole::PROSPECTOR, false, 0});
            role_state.push_back({PlayerRole::PROSPECTOR_2, false, 0});
        }

        switch (player_count) {
            case 3:
                colonist_supply = 55;
                victory_points_supply = 75;
                break;
            case 4:
                colonist_supply = 75;
                victory_points_supply = 100;
                break;
            case 5:
                colonist_supply = 95;
                victory_points_supply = 122;
                break;
        }

        colonist_ship = player_count;
        colonist_supply -= colonist_ship;

        good_supply = {
            {Good::CORN, 10},
            {Good::INDIGO, 11},
            {Good::SUGAR, 11},
            {Good::TOBACCO, 9},
            {Good::COFFEE, 9}
        };

        int corn_count = 10, indigo_count = 12;

        switch (player_count) {
            case 3:
                corn_count -= 1;
                indigo_count -= 2;
                break;
            case 4:
                corn_count -= 2;
                indigo_count -= 2;
                break;
            case 5:
                corn_count -= 2;
                indigo_count -= 3;
                break;
        }

        plantation_supply.insert(plantation_supply.end(), corn_count, Plantation::CORN);
        plantation_supply.insert(plantation_supply.end(), indigo_count, Plantation::INDIGO);
        plantation_supply.insert(plantation_supply.end(), 11, Plantation::SUGAR);
        plantation_supply.insert(plantation_supply.end(), 9, Plantation::TOBACCO);
        plantation_supply.insert(plantation_supply.end(), 8, Plantation::COFFEE);

        std::shuffle(plantation_supply.begin(), plantation_supply.end(), std::mt19937(std::random_device()()));

        // move last 4/5/6 plantations from supply to plantation_offer
        plantation_offer.insert(plantation_offer.end(), std::make_move_iterator(plantation_supply.end() - player_count - 1), std::make_move_iterator(plantation_supply.end()));

        // Building Format: {{cost (1-10), max_discount (1-4), victory_points (1-4), worker capacity (1-3), good produced}, supply_count (1-4)}
        building_supply = {
            {{BuildingType::SMALL_INDIGO_PLANT, 1, 1, 1, 1, Good::INDIGO}, 4},
            {{BuildingType::SMALL_SUGAR_MILL, 2, 1, 1, 1, Good::SUGAR}, 4},
            {{BuildingType::SMALL_MARKET, 1, 1, 1}, 2},
            {{BuildingType::HACIENDA, 2, 1, 1}, 2},
            {{BuildingType::CONSTRUCTION_HUT, 2, 1, 1}, 2},
            {{BuildingType::SMALL_WAREHOUSE, 3, 1, 1}, 2},

            {{BuildingType::LARGE_INDIGO_PLANT, 3, 2, 2, 3, Good::INDIGO}, 3},
            {{BuildingType::LARGE_SUGAR_MILL, 4, 2, 2, 3, Good::SUGAR}, 3},
            {{BuildingType::HOSPICE, 4, 2, 2}, 2},
            {{BuildingType::OFFICE, 5, 2, 2}, 2},
            {{BuildingType::LARGE_MARKET, 5, 2, 2}, 2},
            {{BuildingType::LARGE_WAREHOUSE, 6, 2, 2}, 2},

            {{BuildingType::TOBACCO_STORAGE, 5, 3, 3, 3, Good::TOBACCO}, 3},
            {{BuildingType::COFFEE_ROASTER, 6, 3, 3, 2, Good::COFFEE}, 3},
            {{BuildingType::FACTORY, 7, 3, 3}, 2},
            {{BuildingType::UNIVERSITY, 8, 3, 3}, 2},
            {{BuildingType::HARBOR, 8, 3, 3}, 2},
            {{BuildingType::WHARF, 9, 3, 3}, 2},

            {{BuildingType::GUILD_HALL, 10, 4, 4}, 1},
            {{BuildingType::CUSTOMS_HOUSE, 10, 4, 4}, 1},
            {{BuildingType::CITY_HALL, 10, 4, 4}, 1},
            {{BuildingType::RESIDENCE, 10, 4, 4}, 1},
            {{BuildingType::FORTRESS, 10, 4, 4}, 1}
        };

        ships = {
            {player_count + 1, Good::CORN, 0},
            {player_count + 2, Good::CORN, 0},
            {player_count + 3, Good::CORN, 0}
        };

        trading_house.reserve(4);
        player_state.reserve(player_count);

        for (int player_idx = 0; player_idx < player_count; player_idx++) {
            player_state.emplace_back(player_count, player_idx);
        }
    }

    ~GameState() = default;

    std::vector<Action> get_legal_actions() {
        const auto& player = player_state[current_player_idx];

        if (current_role == PlayerRole::NONE) {
            std::vector<Action> actions;

            for (const auto& role : role_state) {
                if (role.taken)
                    continue;

                // TODO: generalize this and save 20 lines of code

                if (role.role == PlayerRole::PROSPECTOR) {
                    actions.emplace_back(PlayerRole::PROSPECTOR);
                }
                else if (role.role == PlayerRole::PROSPECTOR_2) {
                    actions.emplace_back(PlayerRole::PROSPECTOR_2);
                }
                else if (role.role == PlayerRole::BUILDER) {
                    auto build_actions = get_legal_build_actions(player, 1);
                    actions.insert(actions.end(), build_actions.begin(), build_actions.end());
                }
                else if (role.role == PlayerRole::SETTLER) {
                    auto settler_actions = get_legal_settle_actions(player, true);
                    actions.insert(actions.end(), settler_actions.begin(), settler_actions.end());
                }
                else if (role.role == PlayerRole::CRAFTSMAN) {
                    auto craftsman_actions = get_legal_craftsman_actions(player);
                    actions.insert(actions.end(), craftsman_actions.begin(), craftsman_actions.end());
                }
                else if (role.role == PlayerRole::TRADER) {
                    auto trader_actions = get_legal_trade_actions(player, true);
                    actions.insert(actions.end(), trader_actions.begin(), trader_actions.end());
                }
                else if (role.role == PlayerRole::MAYOR) {
                    auto mayor_actions = get_legal_mayor_actions(player, true);
                    actions.insert(actions.end(), mayor_actions.begin(), mayor_actions.end());
                }
                else if (role.role == PlayerRole::CAPTAIN) {
                    auto captain_actions = get_legal_captain_actions(player, true);
                    actions.insert(actions.end(), captain_actions.begin(), captain_actions.end());
                }

            }

            return actions;
        }
        else if (current_role == PlayerRole::BUILDER) {
            return get_legal_build_actions(player);
        }
        else if (current_role == PlayerRole::SETTLER) {
            return get_legal_settle_actions(player);
        }
        else if (current_role == PlayerRole::TRADER) {
            return get_legal_trade_actions(player);
        }
        else if (current_role == PlayerRole::MAYOR) {
            return get_legal_mayor_actions(player);
        }
        else if (current_role == PlayerRole::CAPTAIN) {
            return get_legal_captain_actions(player);
        }
        // CRAFTSMAN, PROSPECTOR, PROSPECTOR_2 deliberately ommited - they are one-player Roles that trigger instantly

        throw std::runtime_error("Role Not implemented - cannot choose action");
    }

    std::vector<Action> get_legal_build_actions(const PlayerState& player, int discount = 0) {
        std::vector<Action> actions;

        int doubloons = player.doubloons;
        int quarries = player.get_querry_count();

        for (const auto& building : building_supply) {
            // do not allow building unavailable buildings
            if (building.count == 0)
                continue;

            // do not allow building duplicates
            if (std::find_if(player.buildings.begin(), player.buildings.end(), 
                [&building](const BuildingState& player_building) {
                    return player_building.building.type == building.building.type;
                }) != player.buildings.end())
                continue;

            // do not allow building if not enough space
            int building_size = (building.building.cost == 10) ? 2 : 1;
            if (building_size > player.free_town_space)
                continue;

            // allow building if enough doubloons
            int building_cost = std::max(0, building.building.cost - std::min(building.building.max_discount, quarries) - discount);
            if (building_cost <= doubloons) {
                actions.emplace_back(building.building, building_cost);
            }
        }

        if (actions.empty()) {
            actions.push_back({{BuildingType::NONE, 0, 0, 0, 0}, 0});
        }

        return actions;
    }

    std::vector<Action> get_legal_settle_actions(const PlayerState& player, bool is_settler = false) {
        std::vector<Action> actions;

        bool can_choose_quarry = is_settler;

        for (const auto& building : player.buildings) {
            if (building.building.type == BuildingType::CONSTRUCTION_HUT && building.colonists == 1) {
                can_choose_quarry = true;
                break;
            }
        }

        if (can_choose_quarry && quarry_supply > 0) {
            actions.emplace_back(Plantation::QUARRY);
        }

        // Remove duplicates to prune the search tree
        std::set<Plantation> plantation_set(plantation_offer.begin(), plantation_offer.end());

        for (const auto& plantation : plantation_set) {
            // do not allow settling if not enough space // Redundant check, can be removed
            if (player.plantations.size() >= 12)
                continue;

            actions.emplace_back(plantation);
        }

        if (actions.empty()) {
            actions.emplace_back(Plantation::NONE);
        }

        return actions;
    }

    std::vector<Action> get_legal_craftsman_actions(const PlayerState& player) {
        std::vector<Action> actions;

        std::vector<GoodSupply> producing = player.get_producing_goods();

        for (const auto& good : producing) {
            if (good.count > 0) {
                actions.emplace_back(good.good);
            }
        }

        if (actions.empty()) {
            actions.emplace_back(Good::NONE);
        }

        return actions;
    }

    std::vector<Action> get_legal_trade_actions(const PlayerState& player, bool bonus = false) {
        std::vector<Action> actions;

        actions.emplace_back(Good::NONE, 0); // sell nothing, but no bonus

        if (trading_house.size() == 4)
            return actions;

        bool good_allowed[5] = {true, true, true, true, true};
        bool has_office = false;
        int sale_bonus = bonus;

        for (const auto& building : player.buildings) {
            if (building.colonists != 1)
                continue;

            if (building.building.type == BuildingType::OFFICE) {
                has_office = true;
            }
            else if (building.building.type == BuildingType::SMALL_MARKET) {
                sale_bonus += 1;
            }
            else if (building.building.type == BuildingType::LARGE_MARKET) {
                sale_bonus += 2;
            }
        }

        if (!has_office) {
            for (const auto& good : trading_house) {
                good_allowed[static_cast<int>(good)] = false;
            }
        }

        for (int i = 0; i < 5; i++) {
            if (player.goods[i] > 0 && good_allowed[i]) {
                actions.emplace_back(static_cast<Good>(i), i + sale_bonus); // i == price, 0/1/2/3/4 for Corn/Indigo/Sugar/Tobacco/Coffee
            }
        }

        return actions;
    }

    std::vector<Action> get_legal_mayor_actions(const PlayerState& player, bool mayor = false) {
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

        if (mayor) {
            
            colonists_for_player[current_player_idx] += (mayor && colonist_supply > 0); // TODO: Mayor bonus could be refused, but we always accept it for now

            for (int i = 0; i < colonist_ship; i++) {
                colonists_for_player[(current_player_idx + i) % player_count] += 1;
            }
        }

        int total_colonists = player.get_total_colonists() + colonists_for_player[current_player_idx];
        
        std::vector<BuildingType> nonprod_buildings;
        for (const auto& building : player.buildings) {
            if (building.building.good_produced == Good::NONE)
                nonprod_buildings.push_back(building.building.type);
        }

        auto max_goods = player.get_producing_goods(true); // theoretical maximum goods produced
        int querries = player.get_querry_count();
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

        int max_allocs_per_dist = std::min(20u, 2 * DISTRIBUTION_LIMIT / distributions.size()); // arbitrary limit

        for (const auto& dist : distributions) {
            int total_plantation = dist.corn() + 2 * dist.indigo() + 2 * dist.sugar() + 2 * dist.tobacco() + 2 * dist.coffee() + dist.querry();
            int total_building = total_colonists - total_plantation;
            int total_extra = std::max(0, total_building - int(nonprod_buildings.size()));

            for (int i = 0; i < max_allocs_per_dist; i++) {
                auto buildings_copy = nonprod_buildings;
                std::random_shuffle(buildings_copy.begin(), buildings_copy.end()); // randomly choose remaining buildings
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

    std::vector<Action> get_legal_captain_actions(const PlayerState& player, bool captain = false) {
        std::vector<Action> actions;

        // TODO: Wharf

        for (const auto& ship : ships) {
            for (int i = 0; i < 5; i++) {
                Good g = static_cast<Good>(i);

                // do not allow shipping the same Good on more ships
                for (const auto& other_ship : ships) {
                    if (other_ship.good == g && other_ship.good_count > 0 && ship.capacity != other_ship.capacity) {
                        goto next_good;
                    }
                }

                if (player.goods[i] > 0 && ship.capacity - ship.good_count > 0 && (ship.good_count == 0 || ship.good == g)) {
                    actions.emplace_back(ship.capacity, g, captain);
                }

                next_good:;
            }
        }

        if (actions.empty()) {
            actions.emplace_back(0, Good::NONE, captain);
        }

        return actions;
    }

    void perform_action(const Action& action) {
        current_role = action.type;

        auto& player = player_state[current_player_idx];
        int role_idx = static_cast<int>(action.type);
        auto& role = role_state[role_idx];

        if (verbose && role.doubloons > 0)
            std::cout << "Player " << player.player_idx << " got " << role.doubloons << " extra doubloons" << std::endl;

        role.taken = true;
        player.doubloons += role.doubloons;
        role.doubloons = 0;

        if (action.type == PlayerRole::PROSPECTOR) {
            player_state[player.player_idx].doubloons += 1;
            if (verbose)
                std::cout << "Player " << player.player_idx << " chose Prospector and got 1 doubloon" << std::endl;
            next_round();
        }
        else if (action.type == PlayerRole::PROSPECTOR_2) {
            player_state[player.player_idx].doubloons += 1;
            if (verbose)
                std::cout << "Player " << player.player_idx << " chose Prospector(2) and got 1 doubloon" << std::endl;
            next_round();
        }
        else if (action.type == PlayerRole::CRAFTSMAN) {
            if (verbose)
                std::cout << "Player " << player.player_idx << " chose Craftsman:" << std::endl;

            for (int i = 0; i < player_count; i++) {
                int pidx = (current_player_idx + i) % player_count;

                auto& player = player_state[pidx];
                auto producing = player.get_producing_goods();

                if (verbose)
                    std::cout << "Player " << pidx << " got: ";

                for (const auto& produces : producing) {
                    int gidx = static_cast<int>(produces.good);
                    int production_count = std::min(produces.count, good_supply[gidx].count);

                    if (verbose)
                        std::cout << production_count << " " << GoodNames[gidx] << ", ";
                    
                    player.goods[gidx] += production_count;
                    good_supply[gidx].count -= production_count;
                }

                if (verbose)
                    std::cout << std::endl;
            }

            if (action.good != Good::NONE) {
                int gidx = static_cast<int>(action.good);
                int bonus_production_count = std::min(1, good_supply[gidx].count);
                player.goods[gidx] += bonus_production_count;
                good_supply[gidx].count -= bonus_production_count;

                if (verbose && bonus_production_count > 0)
                    std::cout << "Player " << current_player_idx << " got a bonus 1 " << GoodNames[gidx] << std::endl;
            }

            if (verbose)
                std::cout << std::endl;

            next_round();
        }
        else if (action.type == PlayerRole::BUILDER) {
            if (verbose && current_player_idx == current_round_player_idx)
                std::cout << "Player " << current_player_idx << " chose Builder:" << std::endl;

            if (action.building.type != BuildingType::NONE) {
                player.buildings.push_back({action.building, 0});
                player.doubloons -= action.building_cost;
                player.free_town_space -= (action.building.cost == 10) ? 2 : 1;

                auto bit = std::find_if(building_supply.begin(), building_supply.end(), 
                    [&action](const BuildingSupply& building) {
                        return building.building.type == action.building.type;
                    });

                if (bit == building_supply.end() || bit->count <= 0)
                    throw std::runtime_error("Chosen building not found in building supply");

                bit->count--; // one less instance of this building available

                if (verbose)
                    std::cout << "Player " << current_player_idx << " built a " << BuildingNames[static_cast<int>(action.building.type)]
                        << " for " << action.building_cost << " doubloons" << std::endl;

                if (player.free_town_space == 0)
                    trigger_game_end("A Town has been built to completion");
            }

            next_player();
        }
        else if (action.type == PlayerRole::SETTLER) {
            if (verbose && current_player_idx == current_round_player_idx)
                std::cout << "Player " << current_player_idx << " chose Settler:" << std::endl;

            if (action.plantation != Plantation::NONE) {
                player.plantations.push_back({action.plantation, 0});

                // TODO: implement Hacienda, Hospice, Construction Hut triggers for Settler phase

                if (action.plantation == Plantation::QUARRY) {
                    quarry_supply--;
                } else {
                    auto pit = std::find(plantation_offer.begin(), plantation_offer.end(), action.plantation);
                    if (pit == plantation_offer.end())
                        throw std::runtime_error("Chosen plantation not found among face-up plantation tiles");
                    plantation_offer.erase(pit);
                }

                if (verbose)
                    std::cout << "Player " << current_player_idx << " settled a new " << PlantationNames[static_cast<int>(action.plantation)] << " tile " << std::endl;
            }

            next_player();
        }
        else if (action.type == PlayerRole::TRADER) {
            if (verbose && current_player_idx == current_round_player_idx)
                std::cout << "Player " << current_player_idx << " chose Trader:" << std::endl;

            if (action.good != Good::NONE) {
                int gidx = static_cast<int>(action.good);
                int sale_price = action.sell_price;

                player.goods[gidx] -= 1;
                player.doubloons += sale_price;

                trading_house.push_back(action.good);

                if (verbose) {
                    std::cout << "Player " << current_player_idx << " sold 1 " << GoodNames[gidx] << " for " << sale_price << " doubloons" << std::endl;

                    std::cout << "Trading House now contains: ";
                    for (const auto& good : trading_house) {
                        std::cout << GoodNames[static_cast<int>(good)] << ", ";
                    }
                    std::cout << std::endl;
                }
            }

            next_player();
        }
        else if (action.type == PlayerRole::MAYOR) {
            int took_from_supply = 0;
            if (current_player_idx == current_round_player_idx) {
                if (verbose)
                    std::cout << "Player " << current_player_idx << " chose Mayor:" << std::endl;

                if (colonist_supply > 0) {
                    if (verbose)
                        std::cout << "Player " << current_player_idx << " took 1 extra Colonist from the supply" << std::endl;
                    colonist_supply--;
                    took_from_supply++;
                }
            }

            int took_from_ship = colonists_for_player[player.player_idx] - took_from_supply;
            colonist_ship -= took_from_ship;
            colonists_for_player[player.player_idx] = 0;

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

            if (verbose) {
                std::cout << "Player " << current_player_idx << " distributed Colonists:" << std::endl;

                // TODO: seperate into player.print_colonists()

                std::cout << "Buildings: ";
                for (const auto& building : player.buildings) {
                    std::cout << BuildingNames[static_cast<int>(building.building.type)] << " " << building.colonists << "/" << building.building.capacity << ", ";
                }
                std::cout << std::endl;

                std::cout << "Plantations: ";
                for (const auto& plantation : player.plantations) {
                    std::cout << PlantationNames[static_cast<int>(plantation.plantation)] << " " << plantation.colonists << "/1, ";
                }
                std::cout << std::endl;

                if (player.extra_colonists > 0)
                    std::cout << "Extra Colonists: " << player.extra_colonists << std::endl;
            }
            
            next_player();
        }
        else if (action.type == PlayerRole::CAPTAIN) {
            if (verbose && current_player_idx == current_round_player_idx && action.sell_price > 0) {
                std::cout << "Player " << current_player_idx << " chose Captain:" << std::endl;

                for (int i = 0; i < player_count; i++) {
                    int pidx = (current_player_idx + i) % player_count;
                    std::cout << "Player " << pidx << " has: ";
                    for (int g = 0; g < 5; g++) {
                        std::cout << player_state[pidx].goods[g] << " " << GoodNames[g] << ", ";
                    }
                    std::cout << std::endl;
                }

                std::cout << "Ships contain: ";
                for (const auto& ship : ships) {
                    std::cout << ship.good_count << "/" << ship.capacity << " " << GoodNames[static_cast<int>(ship.good)] << ", ";
                }
                std::cout << std::endl;
            }

            if (action.ship_capacity > 0) {
                auto& ship = *std::find_if(ships.begin(), ships.end(), [&action](const Ship& ship) {
                    return ship.capacity == action.ship_capacity; // TODO: check ownership of ship for Wharf
                });
                // TODO: implement Wharf
                ship.good = action.good;
                int gidx = static_cast<int>(action.good);
                int good_count = std::min(ship.capacity - ship.good_count, player.goods[gidx]);
                
                if (good_count == 0)
                    throw std::runtime_error("No goods to load onto ship");
                
                ship.good_count += good_count;
                player.goods[gidx] -= good_count;
                player.victory_points += good_count + action.sell_price; // 1 VP per good + 1 VP for Captain bonus
                victory_points_supply -= good_count + action.sell_price;

                // TODO: implement Harbor bonus

                if (victory_points_supply <= 0)
                    trigger_game_end("Not enough Victory Points available");

                if (verbose) {
                    std::cout << "Player " << current_player_idx << " loaded " << good_count << " " 
                        << GoodNames[static_cast<int>(action.good)] << " onto Ship of size " << ship.capacity << std::endl;

                    std::cout << "Ships now contain: ";
                    for (const auto& ship : ships) {
                        std::cout << ship.good_count << "/" << ship.capacity << " " << GoodNames[static_cast<int>(ship.good)] << ", ";
                    }
                    std::cout << std::endl;
                }

                cant_ship_counter = 0;
            } else {
                if (verbose)
                    std::cout << "Player " << current_player_idx << " can't ship any more goods" << std::endl;

                cant_ship_counter++;
            }

            next_player();
        }

        return;
    }

    void next_governor() {
        // next player becomes Governor, all roles are available
        governor_idx = (governor_idx + 1) % player_count;
        current_round_player_idx = governor_idx;
        current_player_idx = governor_idx;
        round++;

        // reset role state
        for (auto& role : role_state) {
            if (role.taken) {
                role.taken = false;
            }
            else {
                role.doubloons += 1;
            }
        }

        if (game_ending) {
            determine_winner();
        }
    }

    void next_round() {
        // next player has to choose a role

        if (current_role == PlayerRole::SETTLER) {
            // refill plantation offer

            plantation_discard.insert(plantation_discard.end(), plantation_offer.begin(), plantation_offer.end());
            plantation_offer.clear();

            while (plantation_supply.size() > 0 && plantation_offer.size() < player_count + 1) {
                plantation_offer.push_back(plantation_supply.back());
                plantation_supply.pop_back();
            }

            if (plantation_supply.size() == 0) {
                plantation_supply.insert(plantation_supply.end(), plantation_discard.begin(), plantation_discard.end());
                plantation_discard.clear();
                std::shuffle(plantation_supply.begin(), plantation_supply.end(), std::mt19937(std::random_device()()));
            }

            while (plantation_supply.size() > 0 && plantation_offer.size() < player_count + 1) {
                plantation_offer.push_back(plantation_supply.back());
                plantation_supply.pop_back();
            }
        }
        else if (current_role == PlayerRole::MAYOR) {
            // refill colonist ship

            if (colonist_supply > 0) {
                int empty_building_slots = 0;

                for (const auto& player : player_state) {
                    empty_building_slots += player.get_free_town_space();
                }

                int next_colonist_ship = std::max(empty_building_slots, player_count);
                if (next_colonist_ship > colonist_supply)
                    trigger_game_end("Not enough Colonists available to fill the Colonist Ship");
                
                colonist_ship = std::min(next_colonist_ship, colonist_supply);
                colonist_supply -= colonist_ship;

                if (verbose)
                    std::cout << "Colonist Ship refilled with " << colonist_ship << " colonists. Remaining colonist supply: " << colonist_supply << std::endl;
            }
        }
        else if (current_role == PlayerRole::TRADER) {
            // clear trading house

            if (trading_house.size() == 4) {
                for (const auto& good : trading_house) {
                    good_supply[static_cast<int>(good)].count += 1;
                }
                trading_house.clear();

                if (verbose)
                    std::cout << "Trading House was filled then cleared. Goods returned to supply" << std::endl;
            }
        }
        else if (current_role == PlayerRole::CAPTAIN) {
            // clear full ships

            cant_ship_counter = 0;
            for (auto& ship : ships) {
                if (ship.good_count == ship.capacity) {
                    int gidx = static_cast<int>(ship.good);
                    good_supply[gidx].count += ship.good_count;
                    ship.good_count = 0;
                    ship.good = Good::NONE;

                    if (verbose)
                        std::cout << "Ship of size " << ship.capacity << " was filled then cleared. Goods returned to supply" << std::endl;
                }
            }
        }

        current_role = PlayerRole::NONE;
        current_round_player_idx = (current_round_player_idx + 1) % player_count;
        current_player_idx = current_round_player_idx;
        if (current_round_player_idx == governor_idx) {
            next_governor();
        }
    }

    void next_player() {
        // everyone performs an Action of the currrent role
        current_player_idx = (current_player_idx + 1) % player_count;
        if (current_role == PlayerRole::CAPTAIN) {
            if (cant_ship_counter < player_count)
                return;
            else
                next_round();
        }
        else if (current_player_idx == current_round_player_idx) {
            next_round();
        }
    }

    int get_current_player_idx() {
        return current_player_idx;
    }

    bool is_game_over() {
        return (winner != -1);
    }

    int determine_winner() {
        winner = 0;
        int max_points = -1, max_tie = -1;

        for (int i = 0; i < player_count; i++) {
            int points = player_state[i].get_total_victory_points();
            int tie = player_state[i].get_total_goods() + player_state[i].doubloons;
            if (points > max_points || (points == max_points && tie > max_tie)) {
                max_points = points;
                max_tie = tie;
                winner = i;
            }
        }

        return winner;
    }

    void trigger_game_end(const std::string& reason) {
        if (verbose && !game_ending) {
            std::cout << std::endl << "!!!!!" << std::endl;
            std::cout << reason << " - Game will end after this round" << std::endl;
            std::cout << "!!!!!" << std::endl << std::endl;
        }

        game_ending = true;
    }

    void print_all() {
        std::cout << std::endl << "_____GAME STATE_____" << std::endl;

        std::cout << "Ship states: ";

        for (const auto& ship : ships) {
            std::cout << ship.good_count << "/" << ship.capacity << " " << GoodNames[static_cast<int>(ship.good)] << ", ";
        }
        std::cout << std::endl;

        for (const auto& player : player_state) {
            std::cout << "_____PLAYER " << player.player_idx << "_____" << std::endl;
            std::cout << "Doubloons: " << player.doubloons << std::endl;
            std::cout << "VP Chips: " << player.victory_points << std::endl;
            std::cout << "Buildings: ";
            for (const auto& building : player.buildings) {
                std::cout << BuildingNames[static_cast<int>(building.building.type)] << " " << building.colonists << "/" << building.building.capacity << ", ";
            }
            std::cout << std::endl;

            std::cout << "Plantations: ";
            for (const auto& plantation : player.plantations) {
                std::cout << PlantationNames[static_cast<int>(plantation.plantation)] << " " << plantation.colonists << "/1, ";
            }
            std::cout << std::endl;

            std::cout << "Goods: ";
            for (int i = 0; i < 5; i++) {
                std::cout << player.goods[i] << " " << GoodNames[i] << ", ";
            }
            std::cout << std::endl;

            if (player.extra_colonists > 0)
                std::cout << "Extra Colonists: " << player.extra_colonists << std::endl;

            std::cout << "Total Victory Points: " << player.get_total_victory_points() << std::endl;
            std::cout << std::endl << std::endl;
        }

        std::cout << "Round count: " << round << std::endl;

        for (int i = 0; i < player_count; i++) {
            std::cout << "Player " << i << " score: " << player_state[i].get_total_victory_points() << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Player " << determine_winner() << " is the winner!" << std::endl;
    }
};

#endif // GAME_H