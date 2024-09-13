#include "craftsman.h"
#include "game.h"

#include <iostream>

void CraftsmanAction::perform(GameState& g, const Action& action) const {
    auto& player = g.player_state[g.current_player_idx];

    for (int i = 0; i < g.player_count; i++) {
        int pidx = (g.current_player_idx + i) % g.player_count;

        auto& player = g.player_state[pidx];
        auto producing = player.get_producing_goods();

        bool has_factory = false;
        for (const auto& building : player.buildings) {
            if (building.colonists == 0)
                continue;
            if (building.building.type == BuildingType::FACTORY) {
                has_factory = true;
                break;
            }
        }

        if (g.verbose)
            std::cout << "Player " << pidx << " got: ";

        int factory_doubloons = 0;

        for (const auto& produces : producing) {
            int gidx = static_cast<int>(produces.good);
            int production_count = std::min(produces.count, g.good_supply[gidx]);

            if (g.verbose)
                std::cout << production_count << " " << good_name(produces.good) << ", ";
            
            player.goods[gidx] += production_count;
            g.good_supply[gidx] -= production_count;
            if (production_count > 0 && has_factory)
                factory_doubloons += 1;
        }
        if (g.verbose)
            std::cout << std::endl;

        if (has_factory && factory_doubloons > 1) {
            player.doubloons += factory_doubloons - 1;
            if (g.verbose)
                std::cout << "Player " << pidx << " got " << factory_doubloons - 1 << " extra doubloons (Factory)" << std::endl;
        }
    }

    if (action.good != Good::NONE) {
        int gidx = static_cast<int>(action.good);
        int bonus_production_count = std::min(1, g.good_supply[gidx]);
        player.goods[gidx] += bonus_production_count;
        g.good_supply[gidx] -= bonus_production_count;

        if (g.verbose && bonus_production_count > 0)
            std::cout << "Player " << player.idx << " got a bonus 1 " << good_name(action.good) << std::endl;
    }

    if (g.verbose)
        std::cout << std::endl;

    g.next_round();
}

std::vector<Action> CraftsmanAction::get_legal_actions(const GameState& g, bool is_craftsman) const {
    std::vector<Action> actions;

    if (!is_craftsman)
        throw std::runtime_error("Only the Craftsman can perform Craftsman actions");

    const auto& player = g.player_state[g.current_player_idx];

    std::vector<GoodSupply> producing = player.get_producing_goods();

    for (const auto& good : producing) {
        if (good.count > 0) {
            actions.emplace_back(good.good);
        }
    }

    if (actions.empty()) {
        actions.emplace_back(Good::NONE);
    }

    return actions;
}