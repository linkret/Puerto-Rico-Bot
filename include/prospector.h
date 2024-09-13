#ifndef PROSPECTOR_H
#define PROSPECTOR_H

#include "action.h"

class ProspectorAction : public ActionBase {
    static const PlayerRole role = PlayerRole::PROSPECTOR;
public:
    void perform(GameState& game, const Action& action) const override;
    std::vector<Action> get_legal_actions(const GameState& game, bool bonus = false) const override;
};

class Prospector2Action : public ProspectorAction {
    static const PlayerRole role = PlayerRole::PROSPECTOR_2;
};

#endif // Prospector_H