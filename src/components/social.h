#pragma once

#include <cstdint>
#include <vector>

#include <entt/entt.hpp>

namespace ant::components {

struct Faction {
    std::uint32_t faction_id{0};
};

struct SocialConnections {
    std::vector<entt::entity> friends{};
    std::vector<entt::entity> enemies{};
};

// Family unit representation
struct FamilyComponent {
    uint64_t family_id{};
    bool is_head{};
};

// Organization/state/clan representation (this entity represents the collective)
struct OrganizationComponent {
    uint64_t org_id{};
    entt::entity leader{entt::null};
    std::vector<entt::entity> members{};
    float total_wealth{};  // cached for quick access
};

// Marks individual as member of organization
struct MemberComponent {
    uint64_t org_id{};
};

// LLM-controlled entity marker
struct BrainComponent {
    uint64_t llm_id{};
    float cooldown_remaining{};
    bool is_leader{};
};

// ECS singleton state for emergence logic (stored on a dedicated entity)
struct EmergenceState {
    uint64_t next_family_id{1000000};
    uint64_t next_org_id{2000000};
    int org_check_counter{0};
};

} // namespace ant::components
