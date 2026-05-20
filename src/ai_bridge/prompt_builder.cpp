#include "prompt_builder.h"

namespace ant::ai_bridge {

std::string PromptBuilder::BuildPrompt(const core::EcsRegistry& registry, entt::entity entity) {
    auto& reg = registry.Raw();
    
    nlohmann::json payload;
    payload["entity"] = static_cast<std::uint32_t>(entity);

    if (reg.any_of<components::Position>(entity)) {
        const auto& pos = reg.get<components::Position>(entity);
        payload["position"] = {{"x", pos.x}, {"y", pos.y}};
    }

    if (reg.any_of<components::Health>(entity)) {
        const auto& health = reg.get<components::Health>(entity);
        payload["health"] = {{"current", health.current}, {"max", health.max}};
    }

    if (reg.any_of<components::Hunger>(entity)) {
        const auto& hunger = reg.get<components::Hunger>(entity);
        payload["hunger"] = {{"value", hunger.value}, {"decay_rate", hunger.decay_rate}};
    }

    if (reg.any_of<components::Faction>(entity)) {
        const auto& faction = reg.get<components::Faction>(entity);
        payload["faction_id"] = faction.faction_id;
    }

    if (reg.any_of<components::Wealth>(entity)) {
        const auto& wealth = reg.get<components::Wealth>(entity);
        payload["wealth"] = {{"resources", wealth.resources}, {"coins", wealth.coins}};
    }

    if (reg.any_of<components::MemoryRef>(entity)) {
        const auto& memory = reg.get<components::MemoryRef>(entity);
        payload["memory_ref"] = memory.vector_db_id;
    }

    if (reg.any_of<components::SocialConnections>(entity)) {
        const auto& connections = reg.get<components::SocialConnections>(entity);
        payload["social"]["friends"] = connections.friends.size();
        payload["social"]["enemies"] = connections.enemies.size();
    }

    // Enhanced for leader entities: include organization context
    if (reg.any_of<components::BrainComponent>(entity)) {
        const auto& brain = reg.get<components::BrainComponent>(entity);
        payload["brain"]["llm_id"] = brain.llm_id;
        payload["brain"]["is_leader"] = brain.is_leader;

        // If leader, add organization context
        if (brain.is_leader) {
            auto view = reg.view<components::OrganizationComponent>();
            for (auto org_entity : view) {
                auto& org = reg.get<components::OrganizationComponent>(org_entity);
                if (org.leader == entity) {
                    payload["organization"]["org_id"] = org.org_id;
                    payload["organization"]["member_count"] = org.members.size();
                    payload["organization"]["total_wealth"] = org.total_wealth;
                    break;
                }
            }
        }
    }

    return payload.dump();
}

}  // namespace ant::ai_bridge
