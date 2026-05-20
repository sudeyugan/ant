#include "systems/social_sys.h"

#include "core/ecs_registry.h"

namespace ant::systems {

void SocialSystem::Update(core::EcsRegistry& registry, float dt) const {
    (void)registry;
    (void)dt;
}

} // namespace ant::systems
