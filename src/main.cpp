#include <iostream>

#include "game.h"
#include "player.h"

#include "random_strategy.h"
#include "simple_heuristic_strategy.h" // TODO: put all strategies in a single file
#include "maxn_strategy.h"
#include "basic_heuristic.h"
#include "console_strategy.h"

std::vector<int> run_game(std::vector<Strategy*>& strategy, bool verbose = false, int seed = std::random_device()()) {
    int player_count = strategy.size();
    GameState game(player_count, verbose, seed);

    std::vector<Player> players;
    players.reserve(player_count);
    for (int i = 0; i < player_count; i++)
        players.emplace_back(game, strategy[i]);

    while(true) {
        try {
            int player_idx = game.get_current_player_idx();
            players[player_idx].make_move();
            game.check_integrity(); // TODO: seperate config parameter for integrity checks
        } catch (const std::runtime_error& e) {
            game.print_all();
            std::cout << e.what() << std::endl;
            std::cout << "Seed with error: " << seed << std::endl;
            throw e;
        }

        if (game.is_game_over()) {
            if (verbose)
                game.print_all();
            return game.player_placements;
        }
    }
}

std::vector<int> run_random_game(int player_count, Strategy* my_strategy = new RandomStrategy(), bool verbose = false, int seed = std::random_device()()) {
    std::vector<Strategy*> strategies;
    strategies.reserve(player_count);
    strategies.push_back(my_strategy);
    for (int i = 0; i < player_count - 1; i++)
        strategies.push_back(new RandomStrategy());

    return run_game(strategies, verbose, seed);
}

void stress_test_integrity() {
    for (int i = 0; i < 1000; i++) {
        int player_count = rand() % 3 + 3; // 3, 4, 5
        run_random_game(player_count);
    }
    std::cout << "Integrity stress test passed" << std::endl;
}

void measure_winrate() {
    int win_count = 0;
    int game_count = 500; // would prefer 10000 

    for (int i = 0; i < game_count; i++) {
        int player_count = rand() % 3 + 3; // 3, 4, 5
        int my_idx = rand() % player_count;
        
        std::vector<Strategy*> strategies;
        strategies.reserve(player_count);
        
        for (int j = 0; j < player_count; j++) {
            if (j == my_idx)
                strategies.push_back(new SimpleHeuristicStrategy(new BasicHeuristic()));
                //strategies.push_back(new MaxnStrategy(3));
            else
                strategies.push_back(new RandomStrategy());
                //strategies.push_back(new SimpleHeuristicStrategy(new BasicHeuristic()));
        }
    
        auto placements = run_game(strategies);

        if (placements[my_idx] == 0)
            win_count++;
    }

    std::cout << "Winrate: " << 100.0 * win_count / game_count << std::endl;
}

void play_against_computer() {
    std::cout << "How many players do you want to play against?" << std::endl;
    for (int p = 3; p <= 5; p++) {
        std::cout << p << " players" << std::endl;
    }

    auto player_count = ConsoleStrategy::get_user_choice(nullptr, 5, "players", "have in your game", false) + 1;

    std::cout << "What position do you want to play?" << std::endl;

    std::cout << "0: Random position" << std::endl;
    for (int p = 1; p <= player_count; p++) {
        std::cout << p << ": Position " << p << std::endl;
    }

    auto my_idx = ConsoleStrategy::get_user_choice(nullptr, player_count, "player position", "play as", false);
    if (my_idx == -1) {
        my_idx = rand() % player_count;
        std::cout << "Randomly selected position " << my_idx + 1 << std::endl;
    }

    std::vector<Strategy*> strategies;
    strategies.reserve(player_count);
    for (int i = 0; i < player_count; i++) {
        if (i == my_idx)
            strategies.push_back(new ConsoleStrategy());
        else
            strategies.push_back(new MaxnStrategy(5)); // TODO: Make this a parameter to offer multiple difficulties
    }

    std::cout << "Game starting..." << std::endl << std::endl;

    run_game(strategies, true);
}

int main() {
    auto seed = time(0);
    //seed = 0; // Player scores should equal [20, 11, 16, 23] for seed 0 and all RandomStrategies
    srand(seed);
    std::cout << "Seed: " << seed << std::endl;

    play_against_computer();

    //run_random_game(4, new MaxnStrategy(3), true, seed);

    // TODO: Make legit Tests
    //stress_test_integrity(); // Passing

    //measure_winrate();
    // Expected winrate between 3-5 equally skilled players is 26%
    // 1 SimpleHeuristicStrategy vs. N RandomStrategy: 99.5% winrate
    // 1 SimpleHeuristicStrategy vs. N SimpleHeuristicStrategy: 27.3% winrate
    // 1 MaxnStrategy(dep=3) vs. N RandomStrategy: 99.6% winrate
    // 1 MaxnStrategy(dep=3) vs. N SimpleHeuristicStrategy: 27.6% winrate // This isn't as high as I expected, or wanted
    // 1 MaxnStrategy(dep=4) vs. N SimpleHeuristicStrategy: 27.8% winrate // This didn't help much

    // For now, it seems MaxnStrategy isn't really better than SimpleHeuristicStrategy - it just wastes more runtime

    return 0;
}