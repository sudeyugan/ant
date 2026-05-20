#include "systems/survival_sys.h"

#include "components/agent.h"
#include "core/ecs_registry.h"
#include "core/event_bus.h"
#include "core/events.h"

namespace ant::systems {

void SurvivalSystem::Update(core::EcsRegistry& registry, float dt) const {
    auto view = registry.View<components::Hunger>();
    for (auto entity : view) {
        auto& hunger = view.get<components::Hunger>(entity);
        hunger.value -= hunger.decay_rate * dt;
        if (hunger.value <= 0.0f) {
            hunger.value = 0.0f;
            core::EventBus::Get().Publish<core::EventEntityStarved>(entity);
        }
    }
}

} // namespace ant::systems
