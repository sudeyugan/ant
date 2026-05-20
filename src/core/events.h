#pragma once

#include <string>

#include <entt/entt.hpp>

namespace ant::core {

struct EventEntityStarved {
    entt::entity entity{};
};

struct EventEntityDied {
    entt::entity entity{};
};

struct EventCombatResolved {
    entt::entity attacker{};
    entt::entity defender{};
    float damage{0.0f};
};

struct EventLLMDecisionReady {
    entt::entity entity{};
    std::string decision{};
};

} // namespace ant::core
