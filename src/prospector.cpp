#include "prospector.h"
#include "game.h"

#include <iostream>

void ProspectorAction::perform(GameState& g, const Action& action) const {
    auto& player = g.player_state[g.current_player_idx];
    g.player_state[player.idx].doubloons += 1;

    if (g.verbose)
        std::cout << "Player " << player.idx << " got 1 doubloon" << std::endl;
    g.next_round();
}

std::vector<Action> ProspectorAction::get_legal_actions(const GameState& g, bool is_prospector) const {
    if (!is_prospector)
        throw std::runtime_error("Only the Prospector can perform Prospector actions");

    return {Action(role)};
}