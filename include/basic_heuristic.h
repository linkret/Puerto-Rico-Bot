#ifndef BASIC_HEURISTIC_H
#define BASIC_HEURISTIC_H

#include "building.h"
#include "game.h"
#include "state_evaluator.h"

#include <math.h>

class BasicHeuristic : public StateEvaluator {
public:
    std::vector<double> evaluate(const GameState &state) override;
    double building_value(BuildingType type) const;
    double building_score(const Building& building) const;
    int evaluate(const PlayerState &state);
};

#endif // BASIC_HEURISTIC_H
