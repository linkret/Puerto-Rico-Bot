#include "integrity_checker.h"

#include <iostream>
#include <set>

bool GameStateIntegrityChecker::check_integrity() const {
    // check that no invariant has been violated

    int total_colonists = colonist_supply + colonist_ship;
    for (const auto& player : player_state)
        total_colonists += player.get_total_colonists();

    if ((player_count == 3 && total_colonists != 55)
        || (player_count == 4 && total_colonists != 75)
        || (player_count == 5 && total_colonists != 95))
        throw std::runtime_error("Integrity check failed - Total Colonist count is incorrect");

    int goods[5] = {0, 0, 0, 0, 0};
    for (const auto& player : player_state) {
        for (int i = 0; i < 5; i++) {
            goods[i] += player.goods[i];
        }
    }
    for (int i = 0; i < 5; i++) {
        goods[i] += good_supply[i].count;
    }
    for (const auto& ship : ships) {
        if (ship.good != Good::NONE)
            goods[static_cast<int>(ship.good)] += ship.good_count;
    }
    for (const auto& good : trading_house) {
        goods[static_cast<int>(good)] += 1;
    }

    int global_good_supply[5] = {10, 11, 11, 9, 9};
    for (int i = 0; i < 5; i++) {
        if (goods[i] != global_good_supply[i])
            throw std::runtime_error("Integrity check failed - Goods count is incorrect");
    }

    for (const auto& player : player_state) {
        std::vector<BuildingType> buildings;
        for (const auto& building : player.buildings)
            buildings.push_back(building.building.type);
        std::sort(buildings.begin(), buildings.end());
        if (std::unique(buildings.begin(), buildings.end()) != buildings.end())
            throw std::runtime_error("Integrity check failed - Player has duplicate buildings");
    }

    // TODO: check buildings supply

    return true;
}

bool GameStateIntegrityChecker::check_integrity(const GameState& game) {
    return static_cast<const GameStateIntegrityChecker*>(&game)->check_integrity();
}