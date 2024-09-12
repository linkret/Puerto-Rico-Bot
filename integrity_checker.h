#ifndef INTEGRITY_CHECKER_H
#define INTEGRITY_CHECKER_H

#include "game.h"

class GameStateIntegrityChecker : public GameState {
public:
    bool check_integrity() const;
    static bool check_integrity(const GameState& game);
};

#endif // INTEGRITY_CHECKER_H