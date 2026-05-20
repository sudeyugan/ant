#pragma once

#include <string>
#include <cstdint>
#include <vector>

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

// ============ Emergence Events ============

struct EventFamilyFormed {
    uint64_t family_id{};
    entt::entity parent1{entt::null};
    entt::entity parent2{entt::null};
};

struct EventOrganizationFormed {
    uint64_t org_id{};
    entt::entity leader{entt::null};
    uint32_t member_count{};
};

struct EventOrganizationDissolved {
    uint64_t org_id{};
};

struct EventFamilyBroke {
    uint64_t family_id{};
};

struct EventLeadershipTransfer {
    uint64_t org_id{};
    entt::entity old_leader{entt::null};
    entt::entity new_leader{entt::null};
};

// ============ Brain/LLM Events ============

struct EventBrainGranted {
    entt::entity entity{entt::null};
    uint64_t llm_id{};
};

struct EventBrainRevoked {
    entt::entity entity{entt::null};
};

struct EventLLMDecisionMade {
    entt::entity entity{entt::null};
    std::string decision{};
};

// ============ Chronicle Events ============

struct EventChronicleReady {
    std::string report{};
    uint64_t period_tick{};
};

} // namespace ant::core
