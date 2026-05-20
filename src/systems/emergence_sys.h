#pragma once

#include <cstdint>

namespace ant::core {
    class EcsRegistry;
}

namespace ant::spatial {
    class GridHash;
}

namespace ant::components {
    struct EmergenceState;
}

namespace ant::systems {

class EmergenceSystem {
public:
    void Update(ant::core::EcsRegistry& registry, const ant::spatial::GridHash& grid_hash);

private:
    void CheckFamilyEmergence(ant::core::EcsRegistry& registry,
                              const ant::spatial::GridHash& grid_hash,
                              ant::components::EmergenceState& state);
    void CheckOrganizationEmergence(ant::core::EcsRegistry& registry,
                                    const ant::spatial::GridHash& grid_hash,
                                    ant::components::EmergenceState& state);
    void CheckLeadershipTransfer(ant::core::EcsRegistry& registry);
};

}  // namespace ant::systems
