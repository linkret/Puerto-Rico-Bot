#ifndef GOOD_H
#define GOOD_H

#include <string>

enum class Good {
    CORN = 0,
    INDIGO = 1,
    SUGAR = 2,
    TOBACCO = 3,
    COFFEE = 4,
    NONE
};

inline const std::string GoodNames[] = {
    "Corn",
    "Indigo",
    "Sugar",
    "Tobacco",
    "Coffee",
    "None"
};

inline std::string good_name(Good good) { return GoodNames[static_cast<int>(good)]; }

#endif // GOOD_H