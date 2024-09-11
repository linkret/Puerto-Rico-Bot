#include <bits/stdc++.h>

#include "game.h"
#include "player.h"
#include "random_strategy.h"

int main() {
    srand(time(NULL));
    
    // Temporary test cases:
    //srand(3); // 31 21 34 33 currently, tests Factory
    //srand(6); // 20 21 24 21 currently, tests University
    //srand(0); // 25 12 32 26 currently, tests Hacienda
    //srand(1); // 25 12 32 26 currently, tests Hospice

    std::cout << "Hello, World!" << std::endl;

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
        int player_idx = game.get_current_player_idx();
        players[player_idx].make_move();

        if (game.is_game_over()) {
            game.print_all();            
            break;
        }
    }

    return 0;
}