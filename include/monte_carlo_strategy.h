#ifndef MONTE_CARLO_STRATEGY_H
#define MONTE_CARLO_STRATEGY_H

#include "game.h"
#include "player.h"
#include "strategy.h"
#include "random_strategy.h"

#include <vector>
#include <stack>
#include <memory>
#include <random>
#include <cmath>
#include <limits>

class Node {
public:
    Node(const Action& action = Action(), Node* parent = nullptr)
        : action(action), parent(parent), wins(0), visits(0) {}

    Action action; // The action that led to this node
    Node* parent;
    std::vector<std::unique_ptr<Node>> children;
    double wins;
    int visits;
    bool expanded = false;

    // TODO: try changing this weight
    Node* best_child(double exploration_weight = 1.41) const {
        Node* best = nullptr;
        double best_value = -std::numeric_limits<double>::infinity();
        for (const auto& child : children) {
            double uct_value = child->wins / (child->visits + 1e-6) +
                               exploration_weight * std::sqrt(std::log(visits + 1) / (child->visits + 1e-6));
            if (uct_value > best_value) {
                best_value = uct_value;
                best = child.get();
            }
        }
        return best;
    }

    Action best_action() const { //returns move with most visits
        int best_visits = -1;
        Action best_action;
        for (const auto& child : children) {
            if (child->visits > best_visits) {
                best_visits = child->visits;
                best_action = child->action;
            }
        }
        return best_action;
    }
};

// Monte Carlo Tree Search
class MCTSStrategy : public Strategy {
public:
    MCTSStrategy(int iterations = 1000)
        : iterations(iterations), rng(std::random_device{}()) {}

    void make_move(GameState& game) override {
        root = new Node();
        bool verbose = game.verbose;
        game.verbose = false;
        Action action = search(root, game);
        game.verbose = verbose;
        game.perform_action(action);
        delete root;
    }

private:
    int iterations;
    int player_idx = 0;
    Node* root;
    std::mt19937 rng;
    GameState* game = nullptr;

    Action search(Node* root, const GameState& game) {
        player_idx = game.get_current_player_idx();
        for (int i = 0; i < iterations; ++i) {
            GameState game_copy = game;
            this->game = &game_copy;
            Node* node = tree_policy(root);
            double reward = default_policy(node);
            backup(node, reward);

            if (root->children.size() == 1) { // doesn't make sense to continue search if there's only one move
                return root->children[0]->action;
            }
        }
        return root->best_action(); //root->best_child(0)->action;
    }

    Node* tree_policy(Node* node) {
        while (!game->is_game_over()) {
            if (!node->expanded) {
                return expand(node);
            } else {
                node = node->best_child();
                game->perform_action(node->action);
            }
        }
        return node;
    }

    Node* expand(Node* node) {
        auto possible_moves = game->get_legal_actions();

        // Expand the node with all new children
        for (const auto& move : possible_moves) {
            bool move_exists = false;

            for (const auto& child : node->children) { // TODO: fix this shit xD
                if (child->action == move) {
                    move_exists = true;
                    continue;
                }
            }

            if (!move_exists) {
                auto new_node = std::make_unique<Node>(move, node);
                node->children.push_back(std::move(new_node));
            }
        }

        node->expanded = true;

        return node->best_child();
    }

    double default_policy(Node* node) {
        // Random rollout
        Player random_player(*game, new RandomStrategy());

        while(true) {
            try {
                random_player.make_move();
            } catch (const std::runtime_error& e) {
                std::cout << e.what() << std::endl;
                throw e;
            }

            if (game->is_game_over()) {
                return game->winner == player_idx ? 1.0 : 0.0;
                // TODO: can try giving rewards between 0 and 1 depending on how close we got to winning
            }
        }
    }

    void backup(Node* node, double reward) {
        while (node != nullptr) {
            node->visits += 1;
            node->wins += reward;
            node = node->parent;
        }
    }
};

#endif // MONTE_CARLO_STRATEGY_H