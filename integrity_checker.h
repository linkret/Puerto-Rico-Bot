#ifndef INTEGRITY_CHECKER_H
#define INTEGRITY_CHECKER_H

#include "game.h"

#include <stdexcept>

class GameStateIntegrityChecker {
    const GameState& g;
    // Friend class of GameState
public:
    GameStateIntegrityChecker(const GameState& game) : g(game) {}
    bool check_integrity() const;
private:
    void check_colonist_count() const;
    void check_goods_count() const;
    void check_building_duplicate() const;
    void check_building_count() const;
    void check_plantation_count() const;
    void check_victory_points() const;
};

#endif // INTEGRITY_CHECKER_H