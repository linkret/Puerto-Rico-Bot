#ifndef BULDING_H
#define BULDING_H

#include "good.h"

#include <vector>

enum class BuildingType {
    SMALL_INDIGO_PLANT = 0,
    SMALL_SUGAR_MILL = 1,
    SMALL_MARKET = 2,
    HACIENDA = 3,
    CONSTRUCTION_HUT = 4,
    SMALL_WAREHOUSE = 5,

    LARGE_INDIGO_PLANT = 6,
    LARGE_SUGAR_MILL = 7,
    HOSPICE = 8,
    OFFICE = 9,
    LARGE_MARKET = 10,
    LARGE_WAREHOUSE = 11,
    
    TOBACCO_STORAGE = 12,
    COFFEE_ROASTER = 13,
    FACTORY = 14,
    UNIVERSITY = 15,
    HARBOR = 16,
    WHARF = 17,
    
    GUILD_HALL = 18,
    CUSTOMS_HOUSE = 19,
    CITY_HALL = 20,
    RESIDENCE = 21,
    FORTRESS = 22,

    NONE
};

constexpr int BuildingCosts[] = {
    1, 2, 1, 2, 2, 3,
    3, 4, 4, 5, 5, 6,
    5, 6, 7, 8, 8, 9,
    10, 10, 10, 10, 10
};

constexpr int BuildingVPs[] = {
    1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4
}; // same as max_discount for every building

constexpr int BuildingCapacities[] = {
    1, 1, 1, 1, 1, 1,
    3, 3, 1, 1, 1, 1,
    3, 2, 1, 1, 1, 1,
    1, 1, 1, 1, 1
};

constexpr int GlobalSupply[] = {
    4, 4, 2, 2, 2, 2,
    3, 3, 2, 2, 2, 2,
    3, 3, 2, 2, 2, 2,
    1, 1, 1, 1, 1
}; // how many of each building is available at the start of the game

constexpr Good BuildingGoods[] = {
    Good::INDIGO, Good::SUGAR, Good::NONE, Good::NONE, Good::NONE, Good::NONE,
    Good::INDIGO, Good::SUGAR, Good::NONE, Good::NONE, Good::NONE, Good::NONE,
    Good::TOBACCO, Good::COFFEE, Good::NONE, Good::NONE, Good::NONE, Good::NONE,
    Good::NONE, Good::NONE, Good::NONE, Good::NONE, Good::NONE
};

inline const std::string BuildingNames[] = {
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

inline std::string building_name(BuildingType building) { return BuildingNames[static_cast<int>(building)]; }

struct Building {
    BuildingType type;

    Building() : type(BuildingType::NONE) {}
    Building(BuildingType type) : type(type) {}
    Building(const Building& other) : type(other.type) {}

    int cost() const { return BuildingCosts[static_cast<int>(type)]; }
    int victory_points() const { return BuildingVPs[static_cast<int>(type)]; }
    int max_discount() const { return victory_points(); }
    int capacity() const { return BuildingCapacities[static_cast<int>(type)]; }
    Good good_produced() const { return BuildingGoods[static_cast<int>(type)]; }
    int starting_global_supply() const { return GlobalSupply[static_cast<int>(type)]; }
    std::string name() const { return building_name(type); }

    bool operator<(const Building& other) const { return type < other.type; } // for std::set
};

#endif // BULDING_H