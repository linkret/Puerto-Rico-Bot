#ifndef BUILDER_H
#define BUILDER_H

#include "action.h"

class BuilderAction : public ActionBase {
public:
    void perform(GameState& game, const Action& action) const override;
    std::vector<Action> get_legal_actions(const GameState& game, bool is_builder = false) const override;
};

#endif // Builder_H