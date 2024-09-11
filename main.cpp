#include <bits/stdc++.h>

#include "game.h"
#include "player.h"
#include "random_strategy.h"

int main() {
    //srand(time(NULL));
    srand(0); // 37 28 32 43 currently

    std::cout << "Hello, World!" << std::endl;

    int player_count = 4;

    GameState game(player_count, true);

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