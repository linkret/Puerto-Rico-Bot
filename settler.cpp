#include "settler.h"
#include "game.h"

#include <iostream>

void SettlerAction::perform(GameState& g, const Action& action) const {
    auto& player = g.player_state[g.current_player_idx];

    if (action.building_cost > 0) {
        // Hacienda used
        auto random_plantation = g.plantation_supply.back();
        player.plantations.push_back({random_plantation, 0});
        g.plantation_supply.pop_back();

        if (g.verbose)
            std::cout << "Player " << player.idx << " randomly settled a " << plantation_name(random_plantation) << " tile from Hacienda" << std::endl;

        g.hacienda_just_used = true;
        return; // purpesfully don't call next_player() - Hacienda allows for a second (normal) plantation choice
    }

    g.hacienda_just_used = false;

    if (action.plantation != Plantation::NONE) {
        // TODO: implement Hospice triggers for Settler phase
        bool has_hospice = false;
        for (const auto& building : player.buildings) {
            if (building.colonists == 0)
                continue;
            if (building.building.type == BuildingType::HOSPICE) {
                has_hospice = true;
                break;
            }
        }

        if (has_hospice) {
            if (g.colonist_supply > 0) {
                g.colonist_supply -= 1;

                if (g.verbose)
                    std::cout << "Player " << player.idx << " assigned a Colonist to their plantation from the supply because of Hospice" << std::endl;
            }
            else if (g.colonist_ship > 0) {
                g.colonist_ship -= 1;

                if (g.verbose)
                    std::cout << "Player " << player.idx << " assigned a Colonist to their plantation from the ship because of Hospice" << std::endl;
            }
            else 
                has_hospice = false; // no extra Colonist available
        }

        player.plantations.push_back({action.plantation, has_hospice});

        if (action.plantation == Plantation::QUARRY) {
            g.quarry_supply--;
        } else {
            auto pit = std::find(g.plantation_offer.begin(), g.plantation_offer.end(), action.plantation);
            if (pit == g.plantation_offer.end())
                throw std::runtime_error("Chosen plantation not found among face-up plantation tiles");
            g.plantation_offer.erase(pit);
        }

        if (g.verbose)
            std::cout << "Player " << player.idx << " settled a new " << plantation_name(action.plantation) << " tile " << std::endl;
    }

    g.next_player();
}

std::vector<Action> SettlerAction::get_legal_actions(const GameState& g, bool is_settler) const {
    std::vector<Action> actions;

    const auto& player = g.player_state[g.current_player_idx];

    bool can_choose_quarry = is_settler;
    bool has_hacienda = false;

    for (const auto& building : player.buildings) {
        if (building.colonists == 0)
            continue;
        if (building.building.type == BuildingType::CONSTRUCTION_HUT)
            can_choose_quarry = true;
        if (building.building.type == BuildingType::HACIENDA)
            has_hacienda = true;
    }

    if (g.plantation_supply.empty())
        has_hacienda = false; // can't use Hacienda if there are no plantations left

    int pcnt = player.plantations.size();

    if (has_hacienda && !g.hacienda_just_used && pcnt < 12) {
        actions.emplace_back(Plantation::NONE, true); // use Hacienda
        if (pcnt < 11)
            return actions; // always uses Hacienda first if there's at least 2 free spaces left
    }

    if (can_choose_quarry && g.quarry_supply > 0 && pcnt < 12)
        actions.emplace_back(Plantation::QUARRY);

    // Remove duplicates to prune the search tree
    std::set<Plantation> plantation_set(g.plantation_offer.begin(), g.plantation_offer.end());

    for (const auto& plantation : plantation_set) {
        if (pcnt < 12)
            actions.emplace_back(plantation);
    }

    if (actions.empty())
        actions.emplace_back(Plantation::NONE); // we don't do this otherwise, even though it's legal

    return actions;
}