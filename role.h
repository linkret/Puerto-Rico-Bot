#ifndef ROLE_H
#define ROLE_H

#include <string>

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

inline const std::string RoleNames[] = {
    "Mayor",
    "Craftsman",
    "Trader",
    "Settler",
    "Builder",
    "Captain",
    "Prospector",
    "Prospector2",
    "None"
};

inline const std::string role_name(PlayerRole role) { return RoleNames[static_cast<int>(role)]; }

#endif // ROLE_H