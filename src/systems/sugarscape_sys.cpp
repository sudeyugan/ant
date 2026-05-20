#include "systems/sugarscape_sys.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "components/agent.h"
#include "components/transform.h"
#include "core/ecs_registry.h"
#include "core/simulation_config.h"
#include "spatial/grid_hash.h"
#include "spatial/pathfinding.h"
#include "spatial/resource_grid.h"

namespace ant::systems {

namespace {

bool ContainsCell(const std::vector<spatial::GridPoint>& occupied, const spatial::GridPoint& cell) {
    return std::any_of(occupied.begin(), occupied.end(), [&](const spatial::GridPoint& other) {
        return other.x == cell.x && other.y == cell.y;
    });
}

spatial::GridPoint FindBestCell(const spatial::ResourceGrid& grid,
                                const spatial::GridPoint& origin,
                                int radius,
                                const std::vector<spatial::GridPoint>& occupied) {
    if (radius <= 0) {
        return origin;
    }

    float best_sugar = -1.0f;
    spatial::GridPoint best_cell = origin;
    const int min_x = origin.x - radius;
    const int max_x = origin.x + radius;
    const int min_y = origin.y - radius;
    const int max_y = origin.y + radius;

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            if (!grid.InBounds(x, y)) {
                continue;
            }
            const spatial::GridPoint candidate{x, y};
            if (ContainsCell(occupied, candidate)) {
                continue;
            }
            const auto& cell = grid.At(x, y);
            if (cell.current > best_sugar) {
                best_sugar = cell.current;
                best_cell = candidate;
            }
        }
    }

    return best_cell;
}

} // namespace

void SugarscapeSystem::Update(core::EcsRegistry& registry,
                              spatial::ResourceGrid& grid,
                              const spatial::GridHash& hash,
                              const core::SimulationConfig& config,
                              float dt) const {
    (void)dt;
    (void)config;
    if (grid.Width() == 0 || grid.Height() == 0) {
        return;
    }
    auto view = registry.View<components::Position, components::Velocity, components::Hunger, components::Vision>();
    std::vector<entt::entity> neighbors;
    std::vector<spatial::GridPoint> occupied;

    for (auto entity : view) {
        auto& position = view.get<components::Position>(entity);
        auto& velocity = view.get<components::Velocity>(entity);
        auto& hunger = view.get<components::Hunger>(entity);
        const auto& vision = view.get<components::Vision>(entity);

        auto cell = grid.WorldToCell(position);
        if (!grid.InBounds(cell.x, cell.y)) {
            cell.x = std::clamp(cell.x, 0, static_cast<int>(grid.Width() - 1));
            cell.y = std::clamp(cell.y, 0, static_cast<int>(grid.Height() - 1));
            position = grid.CellToWorld(cell);
            velocity = components::Velocity{};
            continue;
        }

        neighbors.clear();
        occupied.clear();
        hash.QueryRadius(position, static_cast<float>(vision.range), neighbors);
        for (auto neighbor : neighbors) {
            if (neighbor == entity || !registry.Raw().valid(neighbor)) {
                continue;
            }
            const auto& neighbor_pos = registry.GetComponent<components::Position>(neighbor);
            occupied.push_back(grid.WorldToCell(neighbor_pos));
        }

        const auto target = FindBestCell(grid, cell, vision.range, occupied);

        if (target.x == cell.x && target.y == cell.y) {
            const float needed = std::max(0.0f, hunger.max - hunger.value);
            if (needed > 0.0f) {
                const float taken = grid.Consume(cell.x, cell.y, needed);
                hunger.value = std::min(hunger.value + taken, hunger.max);
            }
            velocity = components::Velocity{};
            continue;
        }

        const auto path = spatial::Pathfinding::AStar(grid.WalkableMap(), cell, target);
        if (path.size() < 2) {
            velocity = components::Velocity{};
            continue;
        }

        const auto next_world = grid.CellToWorld(path[1]);
        const float dx = next_world.x - position.x;
        const float dy = next_world.y - position.y;
        const float len = std::sqrt(dx * dx + dy * dy);
        if (len > 0.001f) {
            const float speed = 2.0f;
            velocity.dx = (dx / len) * speed;
            velocity.dy = (dy / len) * speed;
        } else {
            velocity = components::Velocity{};
        }
    }
}

} // namespace ant::systems
