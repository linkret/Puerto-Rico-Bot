#ifndef SETTLER_H
#define SETTLER_H

#include "action.h"

class SettlerAction : public ActionBase {
public:
    void perform(GameState& game, const Action& action) const override;
    std::vector<Action> get_legal_actions(const GameState& game, bool is_settler = false) const override;
};

#endif // SETTLER_H