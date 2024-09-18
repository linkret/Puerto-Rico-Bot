#include "console_strategy.h"

#include <iostream>
#include <vector>

void ConsoleStrategy::make_move(GameState& g) {
    Action action;

    auto role = g.current_role;
    if (role == PlayerRole::NONE)
        role = choose_role(g);

    if (role == PlayerRole::PROSPECTOR || role == PlayerRole::PROSPECTOR_2) {
        action = Action(role);
        g.perform_action(action);
        return;
    }

    auto all_actions = g.get_legal_actions();
    
    std::vector<Action> actions;
    for (const auto& a : all_actions) {
        if (a.type == role)
            actions.push_back(a);
    }

    if (actions.empty())
        throw std::runtime_error("No legal actions");

    if (role == PlayerRole::BUILDER)
        action = choose_builder_action(g, actions);
    else if (role == PlayerRole::CRAFTSMAN)
        action = choose_craftsman_action(g, actions);
    else if (role == PlayerRole::TRADER)
        action = choose_trader_action(g, actions);
    else if (role == PlayerRole::CAPTAIN)
        action = choose_captain_action(g, actions);
    else if (role == PlayerRole::SETTLER)
        action = choose_settler_action(g, actions);
    else if (role == PlayerRole::MAYOR)
        action = choose_mayor_action(g, actions);
    else
        throw std::runtime_error("Role Not implemented - cannot choose action");

    g.perform_action(action);
}

int ConsoleStrategy::get_user_choice(const GameState* g, int max, const std::string& noun, const std::string& verb, bool offer_extra) {
    if (offer_extra)
        std::cout << "G or B: Print the entire current game state (G) or just your own board (B)" << std::endl;

    std::cout << "Enter the number of the " << noun << " you want to " << verb << ": ";
    int choice = -1;
    std::string choice_str;

    if (max == 1) {
        std::cout << std::endl << "Only one choice available, selecting it automatically!" << std::endl;
        return 0;
    }

    while (true) {
        std::getline(std::cin, choice_str);

        // TODO: tell the player that empty selection equals the last choice
        if (choice_str.empty()) {
            choice = max - 1;
            break; // ok
        }

        if (offer_extra && (choice_str == "G" || choice_str == "g")) {
            g->print_all(false);
            std::cout << "Enter the number of the " << noun << " you want to " << verb << ": ";
            continue;
        }

        if (offer_extra && (choice_str == "B" || choice_str == "b")) {
            const auto& player = g->player_state[g->current_player_idx];
            player.print_all();
            std::cout << "Enter the number of the " << noun << " you want to " << verb << ": ";
            continue;
        }

        if (!is_number(choice_str)) {
            std::cout << "Invalid " << noun << " choice!" << std::endl;
        } else {
            choice = std::stoi(choice_str) - 1;

            if (choice < 0 || choice >= max)
                std::cout << "Invalid " << noun << " choice!" << std::endl;
            else
                break; // ok
        }
    }

    return choice;
}

PlayerRole ConsoleStrategy::choose_role(GameState& game) const {
    std::cout << "Choose an available role:" << std::endl;
    int i = 0;
    std::vector<PlayerRole> roles;
    for (const auto& role : game.role_state) {
        if (!role.taken) {
            roles.push_back(role.role);
            i++;
            std::cout << i << ": " << role_name(role.role);
            if (role.doubloons > 0) 
                std::cout << " (" << role.doubloons << " doubloons)";
            std::cout << std::endl;
        } else {
            std::cout << "X: " << role_name(role.role) << " (taken)" << std::endl;
        }
    }

    int role_idx = get_user_choice(&game, roles.size(), "role");

    return roles[role_idx];
}

bool ConsoleStrategy::is_number(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

Action ConsoleStrategy::choose_builder_action(GameState& game, const std::vector<Action>& actions) const {
    std::cout << "You have " << game.player_state[game.current_player_idx].doubloons << " doubloons" << std::endl;

    std::cout << "Choose a building to build:" << std::endl;
    int i = 0;
    for (const auto& action : actions) {
        i++;
        std::cout << i << ": " << action.building.name() << " (" << action.building_cost << " doubloons)" << std::endl;
        // TODO: add building descriptions
    }

    int building_idx = get_user_choice(&game, actions.size(), "building", "build");
    
    return actions[building_idx];
}

Action ConsoleStrategy::choose_craftsman_action(GameState& game, const std::vector<Action>& actions) const {
    std::cout << "Choose a bonus Good to produce:" << std::endl;
    int i = 0;
    for (const auto& action : actions) {
        i++;
        std::cout << i << ": " << good_name(action.good) << std::endl;
    }

    int good_idx = get_user_choice(&game, actions.size(), "bonus good");
    
    return actions[good_idx];
}

Action ConsoleStrategy::choose_trader_action(GameState& game, const std::vector<Action>& actions) const {
    std::cout << "Choose a Good to sell:" << std::endl;
    int i = 0;
    for (const auto& action : actions) {
        i++;
        std::cout << i << ": " << good_name(action.good) << " (" << action.sell_price << " doubloons)" << std::endl;
    }

    int good_idx = get_user_choice(&game, actions.size(), "good", "sell");
    
    return actions[good_idx];
}

Action ConsoleStrategy::choose_captain_action(GameState& game, const std::vector<Action>& actions) const {
    if (actions.front().good == Good::NONE)
        return choose_captain_keep_action(game, actions);

    std::cout << "Choose a Good to load onto a Ship:" << std::endl;
    int i = 0;
    for (const auto& action : actions) {
        i++;
        std::cout << i << ": " << good_name(action.good) << " on ship with capacity: " << action.ship_capacity << std::endl;
    }

    int good_idx = get_user_choice(&game, actions.size(), "Good and Ship", "load");
    
    return actions[good_idx];
}

Action ConsoleStrategy::choose_captain_keep_action(GameState& game, const std::vector<Action>& actions) const {
    const auto& player = game.player_state[game.current_player_idx];
    int total_goods = player.get_total_goods();

    int action_total_goods = 0;
    for (int i = 0; i < 5; i++) {
        action_total_goods += actions.front().mayor_allocation.distribution.w[i];
    }

    // If we can keep all the goods, don't even prompt the Player to decide
    if (action_total_goods == total_goods)
        return actions.front();

    Action action(PlayerRole::CAPTAIN);

    int can_store_types = 0;
    for (const auto& building : player.buildings) {
        if (building.colonists == 0)
            continue;

        if (building.building.type == BuildingType::SMALL_WAREHOUSE) 
            can_store_types += 1;
        if (building.building.type == BuildingType::LARGE_WAREHOUSE)
            can_store_types += 2;
    }

    std::cout << "You can keep " << can_store_types << " types of Goods, and one extra Good of any type." << std::endl;

    for (int it = 0; it <= can_store_types; it++) {
        std::cout << "Choose a Good to keep:" << std::endl;

        int i = 0;
        std::vector<std::pair<int, int>> keep; // good index, count
        for (int gidx = 0; gidx < 5; gidx++) {
            if (player.goods[gidx] > 0) {
                i++;
                int can_keep = (it == can_store_types) ? 1 : player.goods[gidx]; // keep just 1 of any type in last iteration
                keep.push_back({gidx, can_keep});
                std::cout << i << ": Keep " << can_keep << " " << good_name(static_cast<Good>(gidx)) << std::endl;
            }
        }

        int good_idx = get_user_choice(&game, keep.size(), "Good", "keep");
        auto keep_choice = keep[good_idx];
        action.mayor_allocation.distribution.w[keep_choice.first] = keep_choice.second;
    }
    
    return action;
}

Action ConsoleStrategy::choose_settler_action(GameState& game, const std::vector<Action>& actions) const {
    std::cout << "Choose a Plantation to settle:" << std::endl;
    
    int i = 0;
    for (const auto& action : actions) {
        i++;
        if (action.building_cost > 0) // Hacienda
            std::cout << i << ": " << "Use Hacienda to get a bonus random Plantation" << std::endl;
        else 
            std::cout << i << ": " << plantation_name(action.plantation) << std::endl;
    }

    int plantation_idx = get_user_choice(&game, actions.size(), "Plantation", "settle");
    
    return actions[plantation_idx];
}

Action ConsoleStrategy::mayor_action_from_player(const PlayerState& player) const {
    // TODO: this isn't 100% exact. It might put an extra colonist on a random plantation instead of a random production building, even though it hardly matters
    // TODO: this method is way too long and complicated. Need to refactor and change MayorAllocation class, this is a mess
    Action action;
    action.type = PlayerRole::MAYOR;
    auto& alloc = action.mayor_allocation;
    alloc.extra_colonists = player.extra_colonists;

    auto quarries = player.get_querry_count();
    auto producing = player.get_producing_goods();
    int plantation_colonists[6] = {0, 0, 0, 0, 0, 0};
    int building_colonists[6] = {0, 0, 0, 0, 0, 0};

    for (const auto& plantation : player.plantations)
        plantation_colonists[static_cast<int>(plantation.plantation)] += plantation.colonists;

    for (const auto& building : player.buildings) {
        if (building.colonists > 0 && building.building.good_produced() == Good::NONE)
            alloc.buildings.push_back(building.building.type); // non-producing building
        if (building.building.good_produced() != Good::NONE)
            building_colonists[static_cast<int>(building.building.good_produced())] += building.colonists;
    }

    // TODO: producing being GoodSupply is really cancer - it doesn't return Goods with count == 0
    for (const auto& good : producing) {
        int gidx = static_cast<int>(good.good);
        alloc.distribution.w[gidx] = good.count;
    }

    for (int gidx = 0; gidx < 5; gidx++) {
        alloc.extra_colonists += plantation_colonists[gidx] - alloc.distribution.w[gidx];
        if (gidx > 0)
            alloc.extra_colonists += building_colonists[gidx] - alloc.distribution.w[gidx];
    }

    alloc.distribution.querry() = quarries;

    // TODO: don't allow extra colonists if there's still room to employ them

    return action;
}

Action ConsoleStrategy::choose_mayor_action(GameState& game, const std::vector<Action>& actions) const {
    // Special interactive implementation - doesn't use predetermined actions

    auto player = game.player_state[game.current_player_idx]; // copy
    int extra_colonists = actions.front().mayor_allocation.colonists() - player.get_total_colonists();
    player.extra_colonists += extra_colonists;
    Action action = mayor_action_from_player(player);

    while (true) {
        std::cout << "Your current colonist distribution: " << std::endl;
        std::cout << "Extra Colonists: " << player.extra_colonists << std::endl;

        std::cout << "Buildings: ";
        for (const auto& building : player.buildings) {
            std::cout << building.building.name() << " " << building.colonists << "/" << building.building.capacity() << ", ";
        }
        std::cout << std::endl;

        std::cout << "Plantations: ";
        for (const auto& plantation : player.plantations) {
            std::cout << plantation_name(plantation.plantation) << " " << plantation.colonists << "/1, ";
        }
        std::cout << std::endl;

        std::cout << "Choose whether to add or remove colonists:" << std::endl;

        std::cout << "1: Add colonists" << std::endl;
        std::cout << "2: Remove colonists" << std::endl;
        std::cout << "3: Done" << std::endl;

        int choice_idx = get_user_choice(&game, 3, "action", "perform", false);

        if (choice_idx == 2)
            break;

        int delta = choice_idx == 0 ? 1 : -1;

        std::cout << "Choose a workplace to " << (choice_idx == 0 ? "add" : "remove") << " colonists " << (choice_idx == 0 ? "to" : "from") << std::endl;

        int i = 0;
        for (const auto& building : player.buildings) {
            i++;
            std::cout << i << ": " << building.building.name() << " " << building.colonists << "/" << building.building.capacity() << std::endl;
        }

        for (const auto& plantation : player.plantations) {
            i++;
            std::cout << i << ": " << plantation_name(plantation.plantation) << " " << plantation.colonists << "/1" << std::endl;
        }

        int idx = get_user_choice(&game, player.buildings.size() + player.plantations.size(), "workplace", "add or remove colonists from", false);

        if (idx < int(player.buildings.size())) {
            auto& building = player.buildings[idx];
            building.colonists += delta;
            if (building.colonists < 0 || building.colonists > building.building.capacity()) {
                std::cout << "Invalid choice of building! This building cannot have " << building.colonists
                    << " colonists with capacity " << building.building.capacity() << std::endl;
                building.colonists -= delta;
                continue;
            }
        } else {
            auto& plantation = player.plantations[idx - player.buildings.size()];
            plantation.colonists += delta;
            if (plantation.colonists < 0 || plantation.colonists > 1) {
                std::cout << "Invalid choice of plantation! Plantation cannot have " << plantation.colonists << " colonists" << std::endl;
                plantation.colonists -= delta;
                continue;
            }
        }
        player.extra_colonists -= delta;

        action = mayor_action_from_player(player);
    }

    return action;
}