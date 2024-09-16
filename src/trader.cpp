#include "trader.h"
#include "game.h"

#include <iostream>

void TraderAction::perform(GameState& g, const Action& action) const {
    auto& player = g.player_state[g.current_player_idx];

    if (action.good != Good::NONE) {
        int gidx = static_cast<int>(action.good);
        int sale_price = action.sell_price;

        player.goods[gidx] -= 1;
        player.doubloons += sale_price;

        g.trading_house.push_back(action.good);

        if (g.verbose) {
            std::cout << "Player " << player.idx << " sold 1 " << good_name(action.good) << " for " << sale_price << " doubloons" << std::endl;

            std::cout << "Trading House now contains: ";
            for (const auto& good : g.trading_house) {
                std::cout << good_name(good) << ", ";
            }
            std::cout << std::endl;
        }
    }

    g.next_player();
}

std::vector<Action> TraderAction::get_legal_actions(const GameState& g, bool is_trader) const {
    std::vector<Action> actions;

    auto& player = g.player_state[g.current_player_idx];

    actions.emplace_back(Good::NONE, 0); // sell nothing, but no bonus

    if (g.trading_house.size() == 4)
        return actions;

    bool good_allowed[5] = {true, true, true, true, true};
    bool has_office = player.has(BuildingType::OFFICE);
    int sale_bonus = is_trader + (int)player.has(BuildingType::SMALL_MARKET) + 2 * (int)player.has(BuildingType::LARGE_MARKET);

    if (!has_office) {
        for (const auto& good : g.trading_house) {
            good_allowed[static_cast<int>(good)] = false;
        }
    }

    for (int i = 0; i < 5; i++) {
        if (player.goods[i] > 0 && good_allowed[i]) {
            actions.emplace_back(static_cast<Good>(i), i + sale_bonus); // i == price, 0/1/2/3/4 for Corn/Indigo/Sugar/Tobacco/Coffee
        }
    }

    return actions;
}