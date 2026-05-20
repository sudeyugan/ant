#pragma once

#include <cstdint>
#include <vector>

#include <entt/entt.hpp>

namespace ant::components {

struct Faction {
    std::uint32_t faction_id{0};
};

struct Wealth {
    float resources{0.0f};
    float coins{0.0f};
};

struct SocialConnections {
    std::vector<entt::entity> friends{};
    std::vector<entt::entity> enemies{};
};

} // namespace ant::components
