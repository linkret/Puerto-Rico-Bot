#ifndef CAPTAIN_H
#define CAPTAIN_H

#include "action.h"

class CaptainAction : public ActionBase {
public:
    void perform(GameState& game, const Action& action) const override;
    std::vector<Action> get_legal_actions(const GameState& game, bool is_captain = false) const override;
};

#endif // CAPTAIN_H