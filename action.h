#ifndef ACTION_H
#define ACTION_H

#include <vector>

#include "role.h"

class GameState; // forward declarations
class Action;

class ActionBase {
    public:
        ~ActionBase() = default;
        virtual void perform(GameState& game, const Action& action) const = 0; // TODO: take this Action argument away
        virtual std::vector<Action> get_legal_actions(const GameState& game, bool bonus = false) const = 0;
};

#endif // ACTION_H