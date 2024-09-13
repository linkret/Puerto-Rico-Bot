#ifndef GAME_H
#define GAME_H

#include "action.h"
#include "building.h"
#include "good.h"
#include "roles.h"

#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <random>
#include <set>

class GameStateIntegrityChecker; // forward declaration

enum class Plantation {
    CORN = 0,
    INDIGO = 1,
    SUGAR = 2,
    TOBACCO = 3,
    COFFEE = 4,
    QUARRY = 5,
    NONE
};

inline const std::string PlantationNames[] = {
    "Corn",
    "Indigo",
    "Sugar",
    "Tobacco",
    "Coffee",
    "Quarry",
    "None"
};

struct PlantationState {
    Plantation plantation;
    int colonists;
};

inline std::string plantation_name(Plantation plantation) { return PlantationNames[static_cast<int>(plantation)]; }
inline std::string plantation_name(PlantationState ps) { return plantation_name(ps.plantation); }

struct BuildingState {
    Building building;
    int colonists;
};

// TODO: consider adding to_string() methods, or overloading << operator instead of x_name(x) methods

struct BuildingSupply {
    Building building;
    int count;
};

struct GoodSupply {
    Good good;
    int count;
};

struct RoleState {
    PlayerRole role;
    bool taken;
    int doubloons;
};

// TODO: also use for Craftsman
struct ProductionDistribution {
    int w[6]; // corn, indigo, sugar, tobacco, coffee, querry;

    int corn() const { return w[0]; }
    int indigo() const { return w[1]; }
    int sugar() const { return w[2]; }
    int tobacco() const { return w[3]; }
    int coffee() const { return w[4]; }
    int querry() const { return w[5]; }
};

struct MayorAllocation {
    ProductionDistribution distribution = {0, 0, 0, 0, 0, 0};
    std::vector<BuildingType> buildings;
    int extra_colonists = 0;

    MayorAllocation() = default;
    MayorAllocation(ProductionDistribution distribution, std::vector<BuildingType> buildings, int extra_colonists) 
        : distribution(distribution), buildings(buildings), extra_colonists(extra_colonists) {}
};

struct PlayerState {
    const int idx;

    int doubloons;
    int victory_points = 0;
    int extra_colonists = 0;
    int goods[5] = {0, 0, 0, 0, 0};
    std::vector<PlantationState> plantations;
    std::vector<BuildingState> buildings;
    int free_town_space = 12;

    PlayerState(int player_count, int player_idx) : idx(player_idx) {
        plantations.reserve(12);
        buildings.reserve(12);

        doubloons = player_count - 1; // 3/4/5 players start with 2/3/4 doubloons

        // Starting plantation:
        if (player_count == 3) {
            if (player_idx == 2)
                plantations.push_back({Plantation::CORN, 0});
            else
                plantations.push_back({Plantation::INDIGO, 0});

        } else if (player_count == 4) {
            if (player_idx >= 2)
                plantations.push_back({Plantation::CORN, 0});
            else
                plantations.push_back({Plantation::INDIGO, 0});
        } else if (player_count == 5) {
            if (player_idx >= 3)
                plantations.push_back({Plantation::CORN, 0});
            else
                plantations.push_back({Plantation::INDIGO, 0});
        }
    }

    int get_total_goods() const {
        int total_goods = 0;
        for (int i = 0; i < 5; i++) {
            total_goods += goods[i];
        }
        return total_goods;
    }

    int get_total_victory_points() const {
        int total_points = victory_points;
        
        for (const auto& building : buildings) {
            total_points += building.building.victory_points();

            if (building.colonists == 0 || building.building.cost() < 10)
                continue;

            if (building.building.type == BuildingType::RESIDENCE) {
                int filled_plantations = std::max(9, static_cast<int>(plantations.size()));
                total_points += filled_plantations - 5; // 4/5/6/7 points for filled 9 or lower/10/11/12 plantation spaces
            }
            else if (building.building.type == BuildingType::CUSTOMS_HOUSE) {
                total_points += victory_points / 4;
            }
            else if (building.building.type == BuildingType::GUILD_HALL) {
                int guild_points = 0;
                for (const auto& other_building : buildings) {
                    if (other_building.building.good_produced() != Good::NONE)
                        guild_points += (other_building.building.capacity() == 1) ? 1 : 2; // 1/2 points for small/large good-producing buildings
                }
                total_points += guild_points;
            }
            else if (building.building.type == BuildingType::CITY_HALL) {
                int city_points = 0;
                for (const auto& other_building : buildings) {
                    if (other_building.building.good_produced() == Good::NONE)
                        city_points += 1; // 1 point for each non-good-producing building
                }
                total_points += city_points;
            }
            else if (building.building.type == BuildingType::FORTRESS) {       
                total_points += get_total_colonists() / 3; // 1 point for each 3 total colonists
            }
        }

        return total_points;
    }

    int get_total_colonists() const {
        int total_colonists = extra_colonists;
        for (const auto& plantation : plantations) {
            total_colonists += plantation.colonists;
        }
        for (const auto& building : buildings) {
            total_colonists += building.colonists;
        }
        return total_colonists;
    }

    int get_querry_count(bool all_quarries = false) const {
        return std::count_if(plantations.begin(), plantations.end(), [all_quarries](const PlantationState& plantation) {
            return plantation.plantation == Plantation::QUARRY && (all_quarries || plantation.colonists == 1);
        });
    }

    int get_free_town_space() const {
        int free_slots = 0;
        for (const auto& building : buildings) {
            free_slots += building.building.capacity() - building.colonists;
        }
        return free_slots;
    }

    std::vector<GoodSupply> get_producing_goods(bool theoretical_maximum = false) const {
        std::vector<GoodSupply> producing_goods;

        int plantation_production[5] = {0, 0, 0, 0, 0};
        int building_production[5] = {12, 0, 0, 0, 0}; // Corn doesn't need a building

        for (const auto& plantation : plantations) {
            if (plantation.plantation != Plantation::QUARRY) {
                plantation_production[static_cast<int>(plantation.plantation)] += (theoretical_maximum ? 1 : plantation.colonists);
            }
        }

        for (const auto& building : buildings) {
            if (building.building.good_produced() != Good::NONE) {
                int gidx = static_cast<int>(building.building.good_produced());
                building_production[gidx] += (theoretical_maximum ? building.building.capacity() : building.colonists);
            }
        }

        for (int i = 0; i < 5; i++) {
            if (theoretical_maximum || (plantation_production[i] > 0 && building_production[i] > 0)) {
                producing_goods.push_back({static_cast<Good>(i), std::min(plantation_production[i], building_production[i])});
            }
        }

        return producing_goods;
    }
};

struct Ship {
    static const int WHARF_CAPACITY = 100;

    int capacity;
    Good good;
    int good_count;
    int owner = -1;

    bool is_wharf() const { return owner != -1; }
};

enum RuleSet {
    CLASSIC,
    BALANCED // more expensive Factory and Harbor, less expensive Wharf, -1 doubloon for first wheat player, ... ?
    // EXPANSION // extra buildings
};

struct Action {
    // TODO: it would be good if this class were smaller memory-wise - std::variant and switch case, std::get_if<BuilderAction>, etc.
    PlayerRole type;

    Building building;
    Plantation plantation = Plantation::NONE;
    int building_cost;
    Good good = Good::NONE;
    int ship_capacity;
    int sell_price;
    MayorAllocation mayor_allocation;

    Action(PlayerRole type) : type(type) {}
    Action(Building building, int cost) : type(PlayerRole::BUILDER), building(building), building_cost(cost) {}
    Action(Plantation plantation, bool hacienda = false) : type(PlayerRole::SETTLER), plantation(plantation), building_cost(hacienda) {}
    Action(Good good) : type(PlayerRole::CRAFTSMAN), good(good) {}
    Action(Good good, int price) : type(PlayerRole::TRADER), good(good), sell_price(price) {}
    Action(MayorAllocation allocation) : type(PlayerRole::MAYOR), mayor_allocation(allocation) {}
    Action(int ship_capacity, Good good, int bonus) : type(PlayerRole::CAPTAIN), good(good), ship_capacity(ship_capacity), sell_price(bonus) {}
    Action(ProductionDistribution dist, int bonus) // for storing Goods after Captain phase
        : type(PlayerRole::CAPTAIN), good(Good::NONE), sell_price(bonus), mayor_allocation(dist, {}, 0) {}
};

class GameState {
    // Puerto Rico board state for 3-5 players

    int seed;
    std::mt19937 rng;

    const RuleSet rule_set = RuleSet::CLASSIC;
    const int player_count;
    const bool verbose = false;
    bool game_ending = false;

    // TODO: seperate these members into classes, e.g. PlayerManager, RoleManager, SupplyManager, etc.

    int round = 0;
    int governor_idx = 0;
    int current_round_player_idx = 0;
    int current_player_idx = 0;
    int winner = -1;
    std::vector<int> player_placements;
    PlayerRole current_role = PlayerRole::NONE;
    std::vector<RoleState> role_state;

    int colonist_supply;
    int colonist_ship;
    int colonists_for_player[5] = {0, 0, 0, 0, 0}; // how many each player will take durring the Mayor phase
    int victory_points_supply;

    int good_supply[5] = {0, 0, 0, 0, 0};

    int quarry_supply = 8;
    std::vector<Plantation> plantation_supply;
    std::vector<Plantation> plantation_offer;
    std::vector<Plantation> plantation_discard;
    bool hacienda_just_used = false;

    std::vector<BuildingSupply> building_supply;
    
    int cant_ship_counter = 0; // for knowing when to end the Captain phase
    std::vector<Ship> ships;
    std::vector<Good> trading_house;

    std::vector<PlayerState> player_state;

    friend class GameStateIntegrityChecker;
    friend class MayorAction;
    friend class TraderAction;
    friend class CraftsmanAction;
    friend class BuilderAction;
    friend class SettlerAction;
    friend class CaptainAction;
    friend class ProspectorAction;
    friend class Prospector2Action;
public:
    // TODO: make Config struct with all parameters (there will be even more of them in the future)
    GameState(int player_count, bool verbose = false, int seed = std::random_device()()) 
        : player_count(player_count), verbose(verbose)
    {
        rng = std::mt19937(seed);

        if (player_count < 3 || player_count > 5) {
            throw std::invalid_argument("Invalid player count");
        }

        role_state = {
            {PlayerRole::MAYOR, false, 0},
            {PlayerRole::CRAFTSMAN, false, 0},
            {PlayerRole::TRADER, false, 0},
            {PlayerRole::SETTLER, false, 0},
            {PlayerRole::BUILDER, false, 0},
            {PlayerRole::CAPTAIN, false, 0}
        };

        if (player_count >= 4) {
            role_state.push_back({PlayerRole::PROSPECTOR, false, 0});
        }
        if (player_count == 5) {
            role_state.push_back({PlayerRole::PROSPECTOR_2, false, 0});
        }

        switch (player_count) {
            case 3:
                colonist_supply = 55;
                victory_points_supply = 75;
                break;
            case 4:
                colonist_supply = 75;
                victory_points_supply = 100;
                break;
            case 5:
                colonist_supply = 95;
                victory_points_supply = 122;
                break;
        }

        colonist_ship = player_count;
        colonist_supply -= colonist_ship;

        good_supply[0] = 10;
        good_supply[1] = 11;
        good_supply[2] = 11;
        good_supply[3] = 9;
        good_supply[4] = 9;

        int corn_count = (player_count == 3) ? 9 : 8;
        int indigo_count = (player_count < 5) ? 10 : 9;

        plantation_supply.insert(plantation_supply.end(), corn_count, Plantation::CORN);
        plantation_supply.insert(plantation_supply.end(), indigo_count, Plantation::INDIGO);
        plantation_supply.insert(plantation_supply.end(), 11, Plantation::SUGAR);
        plantation_supply.insert(plantation_supply.end(), 9, Plantation::TOBACCO);
        plantation_supply.insert(plantation_supply.end(), 8, Plantation::COFFEE);

        std::shuffle(plantation_supply.begin(), plantation_supply.end(), rng);

        // move last 4/5/6 plantations from supply to plantation_offer
        for (int i = 0; i < player_count + 1; i++) {
            plantation_offer.push_back(plantation_supply.back());
            plantation_supply.pop_back();
        }

        building_supply.reserve(23);
        for (int i = 0; i < static_cast<int>(BuildingType::NONE); i++) {
            auto type = static_cast<BuildingType>(i);
            auto building = Building(type);
            building_supply.push_back({building, building.starting_global_supply()});
        }

        ships = {
            {player_count + 1, Good::NONE, 0},
            {player_count + 2, Good::NONE, 0},
            {player_count + 3, Good::NONE, 0}
        };

        trading_house.reserve(4);
        player_state.reserve(player_count);

        for (int player_idx = 0; player_idx < player_count; player_idx++) {
            player_state.emplace_back(player_count, player_idx);
        }
    }

    ~GameState() = default;

    std::vector<Action> get_legal_actions() {
        if (current_role == PlayerRole::NONE) {
            std::vector<Action> actions;

            for (const auto& role : role_state) {
                if (role.taken)
                    continue;

                // TODO: generalize this and save 20 lines of code

                if (role.role == PlayerRole::PROSPECTOR) {
                    auto legal_actions = ProspectorAction().get_legal_actions(*this, true);
                    actions.insert(actions.end(), legal_actions.begin(), legal_actions.end());
                }
                else if (role.role == PlayerRole::PROSPECTOR_2) {
                    auto legal_actions = ProspectorAction().get_legal_actions(*this, true);
                    actions.insert(actions.end(), legal_actions.begin(), legal_actions.end());
                }
                else if (role.role == PlayerRole::BUILDER) {
                    auto build_actions = BuilderAction().get_legal_actions(*this, true);
                    actions.insert(actions.end(), build_actions.begin(), build_actions.end());
                }
                else if (role.role == PlayerRole::SETTLER) {
                    auto legal_actions = SettlerAction().get_legal_actions(*this, true);
                    actions.insert(actions.end(), legal_actions.begin(), legal_actions.end());
                }
                else if (role.role == PlayerRole::CRAFTSMAN) {
                    auto craftsman_actions = CraftsmanAction().get_legal_actions(*this, true);
                    actions.insert(actions.end(), craftsman_actions.begin(), craftsman_actions.end());
                }
                else if (role.role == PlayerRole::TRADER) {
                    auto legal_actions = TraderAction().get_legal_actions(*this, true);
                    actions.insert(actions.end(), legal_actions.begin(), legal_actions.end());
                }
                else if (role.role == PlayerRole::MAYOR) {
                    auto mayor_actions = MayorAction().get_legal_actions(*this, true);
                    actions.insert(actions.end(), mayor_actions.begin(), mayor_actions.end());
                }
                else if (role.role == PlayerRole::CAPTAIN) {
                    auto legal_actions = CaptainAction().get_legal_actions(*this, true);
                    actions.insert(actions.end(), legal_actions.begin(), legal_actions.end());
                }
            }

            return actions;
        }
        else if (current_role == PlayerRole::BUILDER) {
            return BuilderAction().get_legal_actions(*this);
        }
        else if (current_role == PlayerRole::SETTLER) {
            return SettlerAction().get_legal_actions(*this);
        }
        else if (current_role == PlayerRole::TRADER) {
            return TraderAction().get_legal_actions(*this);
        }
        else if (current_role == PlayerRole::MAYOR) {
            return MayorAction().get_legal_actions(*this);
        }
        else if (current_role == PlayerRole::CAPTAIN) {
            return CaptainAction().get_legal_actions(*this);
        }
        // CRAFTSMAN, PROSPECTOR, PROSPECTOR_2 deliberately ommited - they are one-player Roles that trigger instantly

        throw std::runtime_error("Role Not implemented - cannot choose action");
    }

    void perform_action(const Action& action) {
        current_role = action.type;

        auto& player = player_state[current_player_idx];
        int role_idx = static_cast<int>(action.type);
        auto& role = role_state[role_idx];

        if (verbose && player.idx == current_round_player_idx
            && !(role.role == PlayerRole::SETTLER && hacienda_just_used)
            && !(role.role == PlayerRole::CAPTAIN && action.sell_price == 0)
        ) {
            std::cout << "Player "  << player.idx << " chose role: " << role_name(action.type) << std::endl;
            if (role.doubloons > 0)
                std::cout << "Player " << player.idx << " got " << role.doubloons << " extra doubloons" << std::endl;
        }

        role.taken = true;
        player.doubloons += role.doubloons;
        role.doubloons = 0;

        // TODO: dyanmically dispatch .perform() call
        if (action.type == PlayerRole::PROSPECTOR) {
            ProspectorAction().perform(*this, action);
        }
        else if (action.type == PlayerRole::PROSPECTOR_2) {
            Prospector2Action().perform(*this, action);
        }
        else if (action.type == PlayerRole::CRAFTSMAN) {
            CraftsmanAction().perform(*this, action);
        }
        else if (action.type == PlayerRole::BUILDER) {
            BuilderAction().perform(*this, action);
        }
        else if (action.type == PlayerRole::SETTLER) {
            SettlerAction().perform(*this, action);
        }
        else if (action.type == PlayerRole::TRADER) {
            TraderAction().perform(*this, action);
        }
        else if (action.type == PlayerRole::MAYOR) {
            MayorAction().perform(*this, action);
        }
        else if (action.type == PlayerRole::CAPTAIN) {
            CaptainAction().perform(*this, action);
        }

        return;
    }

    void next_governor() {
        // next player becomes Governor, all roles are available
        governor_idx = (governor_idx + 1) % player_count;
        current_round_player_idx = governor_idx;
        current_player_idx = governor_idx;
        round++;

        if (verbose && !game_ending)
            std::cout << "Player " << governor_idx << " is now the Governor" << std::endl;

        // reset role state
        for (auto& role : role_state) {
            if (role.taken) {
                role.taken = false;
            }
            else {
                role.doubloons += 1;
            }
        }

        if (game_ending) {
            determine_winner();
        }
    }

    void next_round() {
        // next player has to choose a role

        if (verbose)
            std::cout << std::endl;

        if (current_role == PlayerRole::SETTLER) {
            // refill plantation offer

            plantation_discard.insert(plantation_discard.end(), plantation_offer.begin(), plantation_offer.end());
            plantation_offer.clear();

            while (plantation_supply.size() > 0 && plantation_offer.size() < std::size_t(player_count + 1)) {
                plantation_offer.push_back(plantation_supply.back());
                plantation_supply.pop_back();
            }

            if (plantation_supply.size() == 0) {
                plantation_supply.insert(plantation_supply.end(), plantation_discard.begin(), plantation_discard.end());
                plantation_discard.clear();
                std::shuffle(plantation_supply.begin(), plantation_supply.end(), rng);
            }

            while (plantation_supply.size() > 0 && plantation_offer.size() < std::size_t(player_count + 1)) {
                plantation_offer.push_back(plantation_supply.back());
                plantation_supply.pop_back();
            }
        }
        else if (current_role == PlayerRole::MAYOR) {
            // refill colonist ship

            int empty_building_slots = 0;

            for (const auto& player : player_state) {
                empty_building_slots += player.get_free_town_space();
            }

            int next_colonist_ship = std::max(empty_building_slots, player_count);
            if (next_colonist_ship > colonist_supply)
                trigger_game_end("Not enough Colonists available to fill the Colonist Ship");
            
            colonist_ship = std::min(next_colonist_ship, colonist_supply);
            colonist_supply -= colonist_ship;

            if (verbose)
                std::cout << "Colonist Ship refilled with " << colonist_ship << " colonists. Remaining colonist supply: " << colonist_supply << std::endl;
        }
        else if (current_role == PlayerRole::TRADER) {
            // clear trading house

            if (trading_house.size() == 4) {
                for (const auto& good : trading_house) {
                    good_supply[static_cast<int>(good)] += 1;
                }
                trading_house.clear();

                if (verbose)
                    std::cout << "Trading House is full! Goods cleared and returned to supply" << std::endl;
            }
        }
        else if (current_role == PlayerRole::CAPTAIN) {
            // clear full ships

            cant_ship_counter = 0;
            for (auto& ship : ships) {
                if (ship.good_count == ship.capacity || (ship.is_wharf() && ship.good_count > 0)) { // always clear Wharf
                    int gidx = static_cast<int>(ship.good);
                    good_supply[gidx] += ship.good_count;
                    ship.good_count = 0;
                    ship.good = Good::NONE;

                    if (verbose)
                        std::cout << "Ship of size " << ship.capacity << " is full! Goods cleared and returned to supply" << std::endl;
                }
            }
        }

        current_role = PlayerRole::NONE;
        current_round_player_idx = (current_round_player_idx + 1) % player_count;
        current_player_idx = current_round_player_idx;
        if (current_round_player_idx == governor_idx) {
            next_governor();
        }
    }

    void next_player() {
        // everyone performs an Action of the currrent role
        current_player_idx = (current_player_idx + 1) % player_count;
        if (current_role == PlayerRole::CAPTAIN) {
            if (cant_ship_counter < player_count)
                return;
            else
                next_round();
        }
        else if (current_player_idx == current_round_player_idx) {
            next_round();
        }
    }

    int get_current_player_idx() {
        return current_player_idx;
    }

    bool is_game_over() {
        return (winner != -1);
    }

    int determine_winner() {
        winner = 0;

        std::vector<std::pair<std::pair<int, int>, int>> player_scores;

        for (int i = 0; i < player_count; i++) {
            int points = player_state[i].get_total_victory_points();
            int tie = player_state[i].get_total_goods() + player_state[i].doubloons;
            player_scores.push_back({{points, tie}, i});
        }

        std::sort(player_scores.begin(), player_scores.end(), std::greater<std::pair<std::pair<int, int>, int>>());

        // determine player placements

        player_placements.clear();
        player_placements.resize(player_count);

        for (int i = 0; i < player_count; i++) {
            player_placements[player_scores[i].second] = i;
        }

        winner = player_scores[0].second;

        return winner;
    }

    void trigger_game_end(const std::string& reason) {
        if (verbose && !game_ending) {
            std::cout << std::endl << "!!!!!" << std::endl;
            std::cout << reason << " - Game will end after this round" << std::endl;
            std::cout << "!!!!!" << std::endl << std::endl;
        }

        game_ending = true;
    }

    void print_all() {
        std::cout << "_____GAME STATE_____" << std::endl;

        std::cout << "Colonist supply: " << colonist_supply << std::endl;
        std::cout << "Colonist ship: " << colonist_ship << std::endl;
        std::cout << "Victory Point supply: " << victory_points_supply << std::endl;
        
        std::cout << "Goods supply: ";	
        for (int i = 0; i < 5; i++) {
            std::cout << good_supply[i] << " " << GoodNames[i] << ", ";
        }
        std::cout << std::endl;
        
        std::cout << "Plantation offer: ";
        for (const auto& plantation : plantation_offer) {
            std::cout << plantation_name(plantation) << ", ";
        }
        std::cout << std::endl;

        std::cout << "Ship states: ";

        for (const auto& ship : ships) {
            std::cout << ship.good_count << "/" << ship.capacity << " " << good_name(ship.good) << ", ";
        }
        std::cout << std::endl;

        std::cout << "Trading house: ";
        for (const auto& good : trading_house) {
            std::cout << good_name(good) << ", ";
        }
        std::cout << std::endl;

        std::cout << std::endl;
        for (const auto& player : player_state) {
            std::cout << "_____PLAYER " << player.idx << "_____" << std::endl;
            std::cout << "Doubloons: " << player.doubloons << std::endl;
            std::cout << "VP Chips: " << player.victory_points << std::endl;
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

            std::cout << "Goods: ";
            for (int i = 0; i < 5; i++) {
                if (player.goods[i] > 0)
                    std::cout << player.goods[i] << " " << GoodNames[i] << ", ";
            }
            std::cout << std::endl;

            if (player.extra_colonists > 0)
                std::cout << "Extra Colonists: " << player.extra_colonists << std::endl;

            std::cout << "Total Victory Points: " << player.get_total_victory_points() << std::endl;
            std::cout << std::endl;
        }

        std::cout << "Round count: " << round << std::endl;

        for (int i = 0; i < player_count; i++) {
            std::cout << "Player " << i << " score: " << player_state[i].get_total_victory_points() << std::endl;
        }
        std::cout << std::endl;

        if (winner != -1)
            std::cout << "Player " << determine_winner() << " is the winner!" << std::endl;
    }

    bool check_integrity() const;
};

#endif // GAME_H