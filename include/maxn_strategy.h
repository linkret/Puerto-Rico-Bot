#ifndef MAXN_STRATEGY_H
#define MAXN_STRATEGY_H

#include "strategy.h"
#include "state_evaluator.h"
#include "basic_heuristic.h"
#include "game.h"

#include <vector>

class MaxnStrategy : public Strategy {
    int max_depth;
    StateEvaluator* evaluator;
public:
    struct Choice {
        Action action;
        std::vector<double> score;
    };

    // TODO: we could give it a time limit and use iterative deepening to get the best move in the time limit - depth will be variable
    
    MaxnStrategy(int depth, StateEvaluator* evaluator = new BasicHeuristic) : max_depth(depth), evaluator(evaluator) {}
    ~MaxnStrategy() override { delete evaluator; };

    void make_move(GameState& game) override {
        bool verbose = game.verbose;
        game.verbose = false; // don't print Actions in maxn() recursion
        std::vector<Action> actions = game.get_legal_actions();

        Choice best_choice = maxn(game, max_depth); // TODO: make this depth configurable
        game.verbose = verbose; // restore verbosity 

        if (best_choice.action.type == PlayerRole::NONE)
            throw std::runtime_error("No legal actions - game is over");

        game.perform_action(best_choice.action);
    }

    Choice maxn(const GameState& state, int depth) {
        int player_idx = state.get_current_player_idx();

        if (depth == 0 || state.is_game_over()) {
            Choice choice;
            choice.score = evaluator->evaluate(state);
            return choice;
        }

        double max_score = std::numeric_limits<int>::min();
        std::vector<Action> actions = state.get_legal_actions();
        Choice best_choice;

        for (const auto& action : actions) {
            GameState next_state = state;
            next_state.perform_action(action);
            Choice choice = maxn(next_state, depth - 1);

            if (choice.score[player_idx] > max_score) {
                max_score = choice.score[player_idx];
                best_choice.action = action;
                best_choice.score = choice.score;
            }
        }

        return best_choice;
    }
};

#endif // MAXN_STRATEGY_H