#include "integrity_checker.h"

#include <iostream>
#include <set>

bool GameStateIntegrityChecker::check_integrity() const {
    // check that no invariant has been violated

    try {
        check_colonist_count();
        check_goods_count();
        check_building_duplicate();
        check_building_count();
        check_plantation_count();
        check_victory_points();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Integrity check failed: " + std::string(e.what()));
    }

    return true;
}

void GameStateIntegrityChecker::check_colonist_count() const {
    int total_colonists = g.colonist_supply + g.colonist_ship;
    for (const auto& player : g.player_state)
        total_colonists += player.get_total_colonists();

    if ((g.player_count == 3 && total_colonists != 55)
        || (g.player_count == 4 && total_colonists != 75)
        || (g.player_count == 5 && total_colonists != 95))
        throw std::runtime_error("Total Colonist count is incorrect");
}

void GameStateIntegrityChecker::check_goods_count() const {
    int goods[5] = {0, 0, 0, 0, 0};
    for (const auto& player : g.player_state) {
        for (int i = 0; i < 5; i++) {
            goods[i] += player.goods[i];
        }
    }
    for (int i = 0; i < 5; i++) {
        goods[i] += g.good_supply[i];
    }
    for (const auto& ship : g.ships) {
        if (ship.good != Good::NONE)
            goods[static_cast<int>(ship.good)] += ship.good_count;
    }
    for (const auto& good : g.trading_house) {
        goods[static_cast<int>(good)] += 1;
    }

    int global_good_supply[5] = {10, 11, 11, 9, 9};
    for (int i = 0; i < 5; i++) {
        if (goods[i] != global_good_supply[i])
            throw std::runtime_error("Goods count is incorrect");
    }
}

void GameStateIntegrityChecker::check_building_duplicate() const {
    for (const auto& player : g.player_state) {
        std::vector<BuildingType> buildings;
        for (const auto& building : player.buildings)
            buildings.push_back(building.building.type);
        std::sort(buildings.begin(), buildings.end());
        if (std::unique(buildings.begin(), buildings.end()) != buildings.end())
            throw std::runtime_error("Player has duplicate buildings");
    }
}

void GameStateIntegrityChecker::check_building_count() const {

}

void GameStateIntegrityChecker::check_plantation_count() const {

}

void GameStateIntegrityChecker::check_victory_points() const {

}