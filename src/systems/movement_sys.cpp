#include "systems/movement_sys.h"

#include "components/transform.h"
#include "core/ecs_registry.h"

namespace ant::systems {

void MovementSystem::Update(core::EcsRegistry& registry, float dt) const {
    auto view = registry.View<components::Position, components::Velocity>();
    for (auto entity : view) {
        auto& position = view.get<components::Position>(entity);
        const auto& velocity = view.get<components::Velocity>(entity);
        position.x += velocity.dx * dt;
        position.y += velocity.dy * dt;
    }
}

} // namespace ant::systems
