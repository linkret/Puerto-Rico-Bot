#ifndef MAYOR_H
#define MAYOR_H

#include "action.h"

class MayorAction : public ActionBase {
public:
    void perform(GameState& game, const Action& action) const override;
    std::vector<Action> get_legal_actions(const GameState& game, bool is_mayor = false) const override;
};

#endif // MAYOR_H