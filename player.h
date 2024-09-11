#ifndef PLAYER_H   
#define PLAYER_H

#include "game.h"
#include "strategy.h"

class Player {
    GameState& game;
    Strategy* strategy;

public:
    Player(GameState& game, Strategy* strategy) : game(game), strategy(strategy) {}

    void make_move() {
        strategy->make_move(game);
    }
};

#endif // PLAYER_H