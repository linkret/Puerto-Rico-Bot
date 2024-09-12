#include <iostream>

#include "game.h"
#include "player.h"
#include "random_strategy.h"
#include "integrity_checker.h"

int main() {
    auto seed = time(0);
    std::cout << "Seed: " << seed << std::endl;
    srand(seed);

    // TODO: Make Tests

    int player_count = 4;

    GameState game(player_count, true);

    // TODO: allow fixing seeds for RandomStrategy() for reproducibility in Tests
    std::vector<Player> players = {
        Player(game, new RandomStrategy()),
        Player(game, new RandomStrategy()),
        Player(game, new RandomStrategy()),
        Player(game, new RandomStrategy())
    };

    while(true) {
        try {
            int player_idx = game.get_current_player_idx();
            players[player_idx].make_move();
            GameStateIntegrityChecker::check_integrity(game); // disable when not debugging
        } catch (const std::runtime_error& e) {
            game.print_all();
            std::cout << e.what() << std::endl;
            break;
        }

        if (game.is_game_over()) {
            game.print_all();            
            break;
        }
    }

    return 0;
}