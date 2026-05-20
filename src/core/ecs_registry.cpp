#include "core/ecs_registry.h"

namespace ant::core {

Entity EcsRegistry::CreateEntity() {
    return registry_.create();
}

void EcsRegistry::DestroyEntity(Entity entity) {
    registry_.destroy(entity);
}

std::size_t EcsRegistry::EntityCount() const {
    return registry_.storage<entt::entity>()->size();
}

entt::registry& EcsRegistry::Raw() {
    return registry_;
}

const entt::registry& EcsRegistry::Raw() const {
    return registry_;
}

} // namespace ant::core
