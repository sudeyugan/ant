#pragma once

namespace ant::core {
class EcsRegistry;
}

namespace ant::systems {

class SocialSystem final {
public:
    void Update(core::EcsRegistry& registry, float dt) const;
};

} // namespace ant::systems
