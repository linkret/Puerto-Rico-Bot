#include "integrity_checker.h"

#include <iostream>
#include <set>
#include <map>

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
    int goods[5] = {10, 11, 11, 9, 9};
    for (const auto& player : g.player_state) {
        for (int i = 0; i < 5; i++) {
            goods[i] -= player.goods[i];
        }
    }
    for (int i = 0; i < 5; i++) {
        goods[i] -= g.good_supply[i];
    }
    for (const auto& ship : g.ships) {
        if (ship.good != Good::NONE)
            goods[static_cast<int>(ship.good)] -= ship.good_count;
    }
    for (const auto& good : g.trading_house) {
        goods[static_cast<int>(good)] -= 1;
    }

    for (int i = 0; i < 5; i++) {
        if (goods[i] != 0)
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
    for (const auto& player : g.player_state) {
        if (player.buildings.size() > 12)
            throw std::runtime_error("Player has more than 12 buildings");
    }

    std::map<Building, int> building_count;

    for (const auto& player : g.player_state) {
        for (const auto& building : player.buildings) {
            building_count[building.building]++;
        }
    }

    for (const auto& building : g.building_supply) {
        building_count[building.building] += building.count;
    }

    if (building_count.size() != 23)
        throw std::runtime_error("Global number of unique buildings count is incorrect");

    for (const auto& [building, count] : building_count) {
        if (   (building.cost() == 10 && count != 1)
            || (building.good_produced() == Good::NONE && building.cost() != 10 && count != 2)
            || (building.good_produced() != Good::NONE && building.capacity() == 1 && count != 4)
            || (building.good_produced() != Good::NONE && building.capacity() > 1 && count != 3)
        )
            throw std::runtime_error("Invalid global building count");
    }
}

void GameStateIntegrityChecker::check_plantation_count() const {
    int plantations[6] = {10, 12, 11, 9, 8, 8};

    std::vector<Plantation> all_plantations;
    all_plantations.reserve(100);

    for (const auto& player : g.player_state) {
        for (const auto& plantation : player.plantations) {
            all_plantations.push_back(plantation.plantation);
        }
    }

    all_plantations.insert(all_plantations.end(), g.plantation_supply.begin(), g.plantation_supply.end());
    all_plantations.insert(all_plantations.end(), g.plantation_offer.begin(), g.plantation_offer.end());
    all_plantations.insert(all_plantations.end(), g.plantation_discard.begin(), g.plantation_discard.end());

    for (const auto& plantation : all_plantations) {
        int plidx = static_cast<int>(plantation);
        plantations[plidx]--;
    }

    plantations[5] -= g.quarry_supply;

    for (int i = 0; i < 5; i++) {
        if (plantations[i] != 0)
            throw std::runtime_error("Plantation count is incorrect");
    }
}

void GameStateIntegrityChecker::check_victory_points() const {
    int vps = (g.player_count == 3) ? 75 : (g.player_count == 4) ? 100 : 122;
    for (const auto& player : g.player_state) {
        vps -= player.victory_points;
    }

    if (vps != g.victory_points_supply)
        throw std::runtime_error("Victory points count is incorrect");
}