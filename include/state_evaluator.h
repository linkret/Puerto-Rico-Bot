#ifndef STATE_EVALUATOR_H
#define STATE_EVALUATOR_H

#include "game.h"

class StateEvaluator {
public:
    virtual ~StateEvaluator() = default;
    virtual std::vector<double> evaluate(const GameState &state) = 0;

    double evaluate(const GameState &state, int player_idx) {
        return evaluate(state)[player_idx];
    }
};

#endif // STATE_EVALUATOR_H