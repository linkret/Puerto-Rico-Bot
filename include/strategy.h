#ifndef STRATEGY_H
#define STRATEGY_H

#include "game.h"

class Strategy {
public:
    virtual void make_move(GameState& game) = 0;
    virtual ~Strategy() = default;
};

#endif // STRATEGY_H