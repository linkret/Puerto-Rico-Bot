#ifndef TRADER_H
#define TRADER_H

#include "action.h"

class TraderAction : public ActionBase {
public:
    void perform(GameState& game, const Action& action) const override;
    std::vector<Action> get_legal_actions(const GameState& game, bool is_trader = false) const override;
};

#endif // TRADER_H