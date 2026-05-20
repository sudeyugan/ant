#pragma once

#include <cstdint>
#include <string>

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include "components/agent.h"
#include "components/social.h"
#include "components/transform.h"
#include "core/ecs_registry.h"

namespace ant::ai_bridge {

class PromptBuilder final {
public:
    static std::string BuildPrompt(const core::EcsRegistry& registry, entt::entity entity) {
        nlohmann::json payload;
        payload["entity"] = static_cast<std::uint32_t>(entity);

        if (registry.HasComponent<components::Position>(entity)) {
            const auto& pos = registry.GetComponent<components::Position>(entity);
            payload["position"] = {{"x", pos.x}, {"y", pos.y}};
        }

        if (registry.HasComponent<components::Health>(entity)) {
            const auto& health = registry.GetComponent<components::Health>(entity);
            payload["health"] = {{"current", health.current}, {"max", health.max}};
        }

        if (registry.HasComponent<components::Hunger>(entity)) {
            const auto& hunger = registry.GetComponent<components::Hunger>(entity);
            payload["hunger"] = {{"value", hunger.value}, {"decay_rate", hunger.decay_rate}};
        }

        if (registry.HasComponent<components::Faction>(entity)) {
            const auto& faction = registry.GetComponent<components::Faction>(entity);
            payload["faction_id"] = faction.faction_id;
        }

        if (registry.HasComponent<components::Wealth>(entity)) {
            const auto& wealth = registry.GetComponent<components::Wealth>(entity);
            payload["wealth"] = {{"resources", wealth.resources}, {"coins", wealth.coins}};
        }

        if (registry.HasComponent<components::MemoryRef>(entity)) {
            const auto& memory = registry.GetComponent<components::MemoryRef>(entity);
            payload["memory_ref"] = memory.vector_db_id;
        }

        if (registry.HasComponent<components::SocialConnections>(entity)) {
            const auto& connections = registry.GetComponent<components::SocialConnections>(entity);
            payload["social"]["friends"] = connections.friends.size();
            payload["social"]["enemies"] = connections.enemies.size();
        }

        return payload.dump();
    }
};

} // namespace ant::ai_bridge
