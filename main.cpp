#include <iostream>

#include "game.h"
#include "player.h"
#include "random_strategy.h"
#include "integrity_checker.h"

void run_random_game(int player_count, bool verbose, int seed = std::random_device()()) {
    GameState game(player_count, verbose, seed);

    std::vector<Player> players;
    players.reserve(player_count);
    for (int i = 0; i < player_count; i++)
        players.emplace_back(game, new RandomStrategy(seed)); // TODO: allow different strategies

    while(true) {
        try {
            int player_idx = game.get_current_player_idx();
            players[player_idx].make_move();
            game.check_integrity(); // TODO: seperate config parameter for integrity checks
        } catch (const std::runtime_error& e) {
            game.print_all();
            std::cout << e.what() << std::endl;
            std::cout << "Seed: " << seed << std::endl;
            throw e;
            break;
        }

        if (game.is_game_over()) {
            if (verbose)
                game.print_all();
            break;
        }
    }
}

void stress_test_integrity() {
    for (int i = 0; i < 1000; i++) {
        int player_count = rand() % 3 + 3; // 3, 4, 5
        run_random_game(player_count, false);
    }
    std::cout << "Integrity stress test passed" << std::endl;
}

int main() {
    auto seed = time(0);
    //seed = 0; // Player scores should equal [20, 11, 16, 23]
    srand(seed);
    std::cout << "Seed: " << seed << std::endl;

    run_random_game(4, true, seed);

    // TODO: Make legit Tests
    //stress_test_integrity(); // Passing

    return 0;
}