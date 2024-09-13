#include "builder.h"
#include "game.h"

#include <iostream>

void BuilderAction::perform(GameState& g, const Action& action) const {
    auto& player = g.player_state[g.current_player_idx];

    if (action.building.type != BuildingType::NONE) {
        bool has_university = false;
        for (const auto& building : player.buildings) {
            if (building.colonists == 0)
                continue;
            if (building.building.type == BuildingType::UNIVERSITY) {
                has_university = true;
                break;
            }
        }

        // TODO: method in game.h, also used in Settler.cpp for Hospice
        if (has_university) {
            if (g.colonist_supply > 0) {
                g.colonist_supply -= 1;

                if (g.verbose)
                    std::cout << "Player " << player.idx << " assigned a Colonist from the supply because of University" << std::endl;
            }
            else if (g.colonist_ship > 0) {
                g.colonist_ship -= 1;

                if (g.verbose)
                    std::cout << "Player " << player.idx << " assigned a Colonist from the ship because of University" << std::endl;
            }
            else 
                has_university = false; // no extra Colonist
        }

        player.buildings.push_back({action.building, has_university}); // only comes with a Colonist from University
        player.doubloons -= action.building_cost;
        player.free_town_space -= (action.building.cost == 10) ? 2 : 1;

        auto bit = std::find_if(g.building_supply.begin(), g.building_supply.end(), [&action](const BuildingSupply& building) {
            return building.building.type == action.building.type;
        });

        if (bit == g.building_supply.end() || bit->count <= 0)
            throw std::runtime_error("Chosen building not found in building supply");

        bit->count--; // one less instance of this building available

        if (action.building.type == BuildingType::WHARF)
            g.ships.push_back({Ship::WHARF_CAPACITY, Good::NONE, 0, player.idx}); // new private ship with effectively infinite capacity

        if (g.verbose)
            std::cout << "Player " << player.idx << " built a " << building_name(action.building)
                << " for " << action.building_cost << " doubloons" << std::endl;

        if (player.free_town_space == 0)
            g.trigger_game_end("A Town has been built to completion");
    }

    g.next_player();
}

std::vector<Action> BuilderAction::get_legal_actions(const GameState& g, bool is_builder) const {
    std::vector<Action> actions;

    const auto& player = g.player_state[g.current_player_idx];

    int doubloons = player.doubloons;
    int quarries = player.get_querry_count();

    for (const auto& building : g.building_supply) {
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
        int building_cost = std::max(0, building.building.cost - std::min(building.building.max_discount, quarries) - is_builder);
        if (building_cost <= doubloons) {
            actions.emplace_back(building.building, building_cost);
        }
    }

    if (actions.empty()) {
        actions.push_back({{BuildingType::NONE, 0, 0, 0, 0}, 0});
    }

    return actions;
}