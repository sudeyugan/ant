#include "systems/sugar_growback_sys.h"

#include "core/simulation_config.h"
#include "spatial/resource_grid.h"

namespace ant::systems {

void SugarGrowbackSystem::Update(spatial::ResourceGrid& grid, const core::SimulationConfig& config, std::uint64_t tick) const {
    if (config.growback_interval == 0) {
        return;
    }
    if (tick % config.growback_interval == 0) {
        grid.Growback(config.growback_rate);
    }
}

} // namespace ant::systems
