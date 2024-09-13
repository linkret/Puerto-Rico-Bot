#include "game.h"
#include "integrity_checker.h"

bool GameState::check_integrity() const {
    return GameStateIntegrityChecker(*this).check_integrity();
}
