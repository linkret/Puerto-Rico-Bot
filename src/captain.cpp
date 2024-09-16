#include "captain.h"
#include "game.h"

#include <iostream>

void CaptainAction::perform(GameState& g, const Action& action) const {
    auto& player = g.player_state[g.current_player_idx];

    if (g.verbose && g.current_player_idx == g.current_round_player_idx && action.sell_price > 0) {
        for (int i = 0; i < g.player_count; i++) {
            int pidx = (g.current_player_idx + i) % g.player_count;
            std::cout << "Player " << pidx << " has: ";
            for (int j = 0; j < 5; j++) {
                if (g.player_state[pidx].goods[j] > 0)
                    std::cout << g.player_state[pidx].goods[j] << " " << GoodNames[j] << ", ";
            }
            std::cout << std::endl;
        }

        std::cout << "Ships contain: ";
        for (const auto& ship : g.ships) {
            std::cout << ship.good_count << "/" << ship.capacity << " " << good_name(ship.good) << ", ";
        }
        std::cout << std::endl;
    }

    if (action.good != Good::NONE) {
        auto& ship = *std::find_if(g.ships.begin(), g.ships.end(), [&action, &g](const Ship& ship) {
            if (ship.is_wharf() && ship.owner != g.current_player_idx)
                return false;
            return ship.capacity == action.ship_capacity;
        });
        
        ship.good = action.good;
        int gidx = static_cast<int>(action.good);
        int good_count = std::min(ship.capacity - ship.good_count, player.goods[gidx]);
        
        if (good_count == 0)
            throw std::runtime_error("No goods to load onto ship");

        bool has_harbor = false;
        for (const auto& building : player.buildings) {
            if (building.colonists == 0)
                continue;

            if (building.building.type == BuildingType::HARBOR)
                has_harbor = true;
        }
        
        ship.good_count += good_count;
        player.goods[gidx] -= good_count;
        int vps = good_count + action.sell_price + has_harbor; // 1 VP per good + 1 VP for Captain bonus + 1 VP for Harbor bonus
        player.victory_points += vps; 
        g.victory_points_supply -= vps;

        if (g.victory_points_supply <= 0)
            g.trigger_game_end("Not enough Victory Points available");

        if (g.verbose) {
            std::cout << "Player " << g.current_player_idx << " loaded " << good_count << " " 
                << good_name(action.good) << " onto Ship of size " << ship.capacity << std::endl;

            std::cout << "Player " << g.current_player_idx << " got " << vps << " Victory Points" << std::endl;

            std::cout << "Ships now contain: ";
            for (const auto& ship : g.ships) {
                std::cout << ship.good_count << "/" << ship.capacity << " " << good_name(ship.good) << ", ";
            }
            std::cout << std::endl;
        }

        g.cant_ship_counter = 0;
    } else {
        // Can't ship anything else, time to store Goods in Warehouses

        if (g.verbose)
            std::cout << "Player " << g.current_player_idx << " can't ship any more goods" << std::endl;

        auto& kept_goods = action.mayor_allocation.distribution;
        for (int i = 0; i < 5; i++) {
            if (g.verbose && kept_goods.w[i] != player.goods[i])
                std::cout << "Player " << g.current_player_idx << " threw away " << player.goods[i] - kept_goods.w[i] << " " << GoodNames[i] << std::endl;

            g.good_supply[i] += player.goods[i] - kept_goods.w[i];
            player.goods[i] = kept_goods.w[i];
        }

        g.cant_ship_counter++;
    }

    g.next_player();
}

std::vector<Action> CaptainAction::get_legal_actions(const GameState& g, bool is_captain) const {
    std::vector<Action> actions;

    const auto& player = g.player_state[g.current_player_idx];

    bool has_wharf = player.has(BuildingType::WHARF);

    for (const auto& ship : g.ships) {
        if (ship.is_wharf() && !(has_wharf && ship.owner == player.idx))
            continue;

        for (int i = 0; i < 5; i++) {
            Good good = static_cast<Good>(i);

            // do not allow shipping the same Good on more public ships
            for (const auto& other_ship : g.ships) {
                if (ship.is_wharf()) // Can ship anything
                    break;
                if (other_ship.is_wharf()) // Does not limit other ships
                    continue;
                if (other_ship.good == good && other_ship.good_count > 0 && ship.capacity != other_ship.capacity) {
                    goto next_good;
                }
            }

            if (player.goods[i] > 0 && ship.capacity - ship.good_count > 0 && (ship.good_count == 0 || ship.good == good)) {
                actions.emplace_back(ship.capacity, good, is_captain);
            }

            next_good:;
        }
    }

    if (actions.empty()) { // throw away remaining goods, save some in warehouses
        int can_store_types = 0;
        for (const auto& building : player.buildings) {
            if (building.colonists == 0)
                continue;

            if (building.building.type == BuildingType::SMALL_WAREHOUSE) 
                can_store_types += 1;
            if (building.building.type == BuildingType::LARGE_WAREHOUSE)
                can_store_types += 2;
        }

        int good_type_cnt = 0;
        int alone_good_cnt = 0;
        for (int i = 0; i < 5; i++) {
            if (player.goods[i] > 0)
                good_type_cnt++;
            if (player.goods[i] == 1)
                alone_good_cnt++;
        }

        if (good_type_cnt <= can_store_types || (good_type_cnt == 1 + can_store_types && alone_good_cnt > 0)) {
            // can store everything
            actions.emplace_back(ProductionDistribution{player.goods[0], player.goods[1], player.goods[2], player.goods[3], player.goods[4], 0}, is_captain);
        } else {
            // greedily store the most abundant goods - tiebreaker higher value
            int stored_goods[5] = {0, 0, 0, 0, 0};
            std::vector<std::pair<int, int>> goods; // (good_count, good_index)
            for (int i = 0; i < 5; i++) {
                goods.emplace_back(player.goods[i], i);
            }
            std::sort(goods.begin(), goods.end(), std::greater<std::pair<int, int>>());

            for (int i = 0; i < can_store_types; i++) {
                stored_goods[goods[i].second] = player.goods[goods[i].second];
            }

            // store one aditional highest-value good
            for (int i = 4; i >= 0; i--) {
                if (player.goods[i] >= 1 && stored_goods[i] == 0) {
                    stored_goods[i] = 1;
                    break;
                }
            }

            actions.emplace_back(ProductionDistribution{stored_goods[0], stored_goods[1], stored_goods[2], stored_goods[3], stored_goods[4], 0}, is_captain);
        }
    }

    return actions;
}