#include "systems/social_sys.h"

#include "components/agent.h"
#include "components/social.h"
#include "core/ecs_registry.h"
#include "core/event_bus.h"
#include "core/events.h"

namespace ant::systems {

void SocialSystem::Update(core::EcsRegistry& registry, float dt) const {
    auto view = registry.View<components::Health, components::Faction, components::SocialConnections>();
    for (auto entity : view) {
        auto& health = view.get<components::Health>(entity);
        const auto& faction = view.get<components::Faction>(entity);
        const auto& connections = view.get<components::SocialConnections>(entity);

        for (auto enemy : connections.enemies) {
            if (!registry.Raw().valid(enemy) || !registry.HasComponent<components::Faction>(enemy)) {
                continue;
            }

            const auto& enemy_faction = registry.GetComponent<components::Faction>(enemy);
            if (enemy_faction.faction_id == faction.faction_id) {
                continue;
            }

            const float damage = 5.0f * dt;
            health.current -= damage;
            if (health.current < 0.0f) {
                health.current = 0.0f;
            }
            core::EventBus::Get().Publish<core::EventCombatResolved>(entity, enemy, damage);
            break;
        }
    }
}

} // namespace ant::systems
