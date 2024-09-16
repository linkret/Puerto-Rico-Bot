#ifndef SIMPLE_HEURISTIC_STRATEGY_H
#define SIMPLE_HEURISTIC_STRATEGY_H

#include "game.h"
#include "strategy.h"
#include "state_evaluator.h"

#include <numeric>

// TODO: verify it's equivalent to maxn_strategy with depth == 1. If so, we can simplify this file
class SimpleHeuristicStrategy : public Strategy {
    StateEvaluator* evaluator;
public:
    SimpleHeuristicStrategy(StateEvaluator* evaluator) : evaluator(evaluator) {}
    ~SimpleHeuristicStrategy() override { delete evaluator; };

    void make_move(GameState& game) override {
        std::vector<Action> actions = game.get_legal_actions();
        int player_idx = game.get_current_player_idx();

        // TODO: fix minor bug with Hacienda where secret rng information is revealed in the new GameState - need to reshuffle the deck every time

        // Find the action that maximizes the current player's score using the simple heuristic
        Action best_action(PlayerRole::NONE);
        double best_score = std::numeric_limits<int>::min();

        for (const auto& action : actions) {
            GameState next_state = game;
            next_state.perform_action(action);

            // Original simple max score heuristic
            double score = evaluator->evaluate(next_state, player_idx);
            if (score > best_score) {
                best_score = score;
                best_action = action;
            }

            //// TODO: make this score comparison into another Heuristic. 
            //// For example if we're player 0: going into [5,2,2] is obviously better than going into [6,7,8], so just maximising our score is not always the best
            //// Didn't prove any better than the basic strategy
            // double lam = 1.0;
            // auto scores = evaluator->evaluate(nextState);
            // auto score = std::pow(scores[player_idx], lam) * (game.player_count - 1);
            
            // for (int i = 0; i < game.player_count; i++) {
            //     if (i != player_idx) {
            //         score -= std::pow(scores[i], lam);
            //     }
            // }

            // if (score > bestScore) {
            //     bestScore = score;
            //     bestAction = action;
            // }

            //// Max (score - 2. best) heuristic // TODO: lower weights for other players
            // auto scores = evaluator->evaluate(nextState);
            // double second_best = std::numeric_limits<int>::min();
            // for (int i = 0; i < game.player_count; i++) {
            //     if (i != player_idx) {
            //         second_best = std::max(second_best, scores[i]);
            //     }
            // }
            // double score = scores[player_idx] - second_best * 0.8;
            // if (score > bestScore) {
            //     bestScore = score;
            //     bestAction = action;
            // }
        }

        if (best_score == std::numeric_limits<int>::min())
            throw std::runtime_error("No legal actions");

        game.perform_action(best_action);
    }
};

#endif // SIMPLE_HEURISTIC_STRATEGY_H
