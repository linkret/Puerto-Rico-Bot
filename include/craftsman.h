#ifndef CRAFTSMAN_H
#define CRAFTSMAN_H

#include "action.h"

class CraftsmanAction : public ActionBase {
public:
    void perform(GameState& game, const Action& action) const override;
    std::vector<Action> get_legal_actions(const GameState& game, bool is_craftsman = false) const override;
};

#endif // CRAFTSMAN_H