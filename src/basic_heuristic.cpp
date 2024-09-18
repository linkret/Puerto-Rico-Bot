#include "basic_heuristic.h"

#include <math.h>
#include <vector>

std::vector<double> BasicHeuristic::evaluate(const GameState &state) {
    std::vector<double> score(state.player_count, 0.0);

    if (state.is_game_over()) {
        for (int i = 0; i < state.player_count; i++)
            score[i] = (i == state.winner) ? 1000.0 : 0.0;
        return score;
    }

    for (int i = 0; i < state.player_count; i++)
        score[i] = evaluate(state.player_state[i]);
    score[state.get_current_player_idx()] += 1.0;

    return score;
}

double BasicHeuristic::building_value(BuildingType type) const {
    // Note: I think these are bad because it causes the bot to build them even lategame, when VPs are more important

    switch (type) {
        //case BuildingType::SMALL_MARKET: return 0.5;
        //case BuildingType::SMALL_INDIGO_PLANT: return 1.0;
        //case BuildingType::SMALL_SUGAR_MILL: return 1.0;
        // case BuildingType::HACIENDA: return 0.5;
        // case BuildingType::CONSTRUCTION_HUT: return 0.5;
        // case BuildingType::SMALL_WAREHOUSE: return 1.0;
        // case BuildingType::LARGE_MARKET: return 0.5;
        //case BuildingType::TOBACCO_STORAGE: return 0.5;
        //case BuildingType::COFFEE_ROASTER: return 0.5;
        case BuildingType::FACTORY: return 1.0;
        // case BuildingType::HARBOR: return 3.0;
        // case BuildingType::WHARF: return 1.0;
        case BuildingType::GUILD_HALL: return 3.0;
        case BuildingType::CUSTOMS_HOUSE: return 1.0;
        // case BuildingType::CITY_HALL: return 0.5;
        // case BuildingType::RESIDENCE: return 1.0;
        // case BuildingType::FORTRESS: return 1.5;
        default: return 0.0;
    };
}

double BasicHeuristic::building_score(const Building& building) const {
    return 1.0 * building_value(building.type);
}

int BasicHeuristic::evaluate(const PlayerState &state) {
    double score = 0;

    // TODO: probably need to have different weights for different player counts
    // Also different strategies are needed for different game stages. It doesn't make sense to save for a giga Fortress in the start, etc.

    score += state.get_total_victory_points() * 1.0;
    score += state.get_total_goods() * 0.2;
    score += std::sqrt(state.doubloons) * 1.0;
    score += state.doubloons * 0.1;
    score += std::sqrt(state.get_querry_count(true)) * 0.1;

    for (const auto& g : state.get_producing_goods(false)) // real production
        score += g.count * (static_cast<int>(g.good) + 1.0) * 0.33; // more valuable goods are worth more

    for (const auto& g : state.get_producing_goods(true)) // theoretical maximum production
        score += g.count * (static_cast<int>(g.good) + 1.0) * 0.2;
    
    for (const auto& b : state.buildings) {
        auto val = building_score(b.building);
        score += (b.colonists > 0) ? val * 1.0 : val * 0.6; // 0.6 > 99.5%
    }

    return score;
}
