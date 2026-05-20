#pragma once

#include <cstdint>

namespace ant::core {
struct SimulationConfig;
}

namespace ant::spatial {
class ResourceGrid;
}

namespace ant::systems {

class SugarGrowbackSystem final {
public:
    void Update(spatial::ResourceGrid& grid, const core::SimulationConfig& config, std::uint64_t tick) const;
};

} // namespace ant::systems
