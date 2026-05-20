#pragma once

namespace ant::core {
class EcsRegistry;
struct SimulationConfig;
}

namespace ant::spatial {
class GridHash;
class ResourceGrid;
}

namespace ant::systems {

class SugarscapeSystem final {
public:
    void Update(core::EcsRegistry& registry,
                spatial::ResourceGrid& grid,
                const spatial::GridHash& hash,
                const core::SimulationConfig& config,
                float dt) const;
};

} // namespace ant::systems
