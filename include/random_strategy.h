#ifndef RANDOM_STRATEGY_H
#define RANDOM_STRATEGY_H

#include "game.h"
#include "strategy.h"

#include <random>

class RandomStrategy : public Strategy {
    std::mt19937 rng;
public:
    RandomStrategy(int seed = std::random_device()()) : rng(seed) {}
    ~RandomStrategy() override = default;

    void make_move(GameState& game) override {
        std::vector<Action> actions = game.get_legal_actions();
        
        // It is good to avoid overrepresenting Roles that have a higher number of legal Actions (Mayor, Settler, Builder)
        // So we first pick a random Role, then independantly choose an Action belonging to that Role

        std::set<PlayerRole> roles;
        for (const auto& action : actions) {
            roles.insert(action.type);
        }
        std::vector<PlayerRole> role_vector(roles.begin(), roles.end());
        int role_idx = rng() % role_vector.size();
        PlayerRole role = role_vector[role_idx];

        actions.erase(std::remove_if(actions.begin(), actions.end(), [role](const Action& action) {
            return action.type != role;
        }), actions.end());

        if (actions.empty())
            throw std::runtime_error("No legal actions");

        int action_idx = rng() % actions.size();
        game.perform_action(actions[action_idx]);
    }
};

#endif // RANDOM_STRATEGY_H