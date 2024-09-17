#ifndef CONSOLE_STRATEGY_H
#define CONSOLE_STRATEGY_H

#include "game.h"
#include "strategy.h"

class ConsoleStrategy : public Strategy {
public:
    ConsoleStrategy() {}
    ~ConsoleStrategy() override = default;

    void make_move(GameState& game) override;
    static int get_user_choice(const GameState* g = nullptr, int max = 100, const std::string& noun = "", const std::string& verb = "choose", bool offer_extra = true);
    static bool is_number(const std::string& s);
private:
    PlayerRole choose_role(GameState& game) const;
    Action choose_builder_action(GameState& game, const std::vector<Action>& actions) const;
    Action choose_settler_action(GameState& game, const std::vector<Action>& actions) const;
    Action choose_trader_action(GameState& game, const std::vector<Action>& actions) const;
    Action choose_craftsman_action(GameState& game, const std::vector<Action>& actions) const;
    Action choose_captain_action(GameState& game, const std::vector<Action>& actions) const;
    Action choose_captain_keep_action(GameState& game, const std::vector<Action>& actions) const;
    Action choose_mayor_action(GameState& game, const std::vector<Action>& actions) const;
    Action mayor_action_from_player(const PlayerState& player) const;
};

#endif // CONSOLE_STRATEGY_H