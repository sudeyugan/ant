#include "spatial/grid_hash.h"

#include <cmath>

#include "core/ecs_registry.h"

namespace ant::spatial {

GridHash::GridHash(float cell_size) : cell_size_(cell_size) {}

void GridHash::Clear() {
    cells_.clear();
}

void GridHash::Rebuild(const core::EcsRegistry& registry) {
    cells_.clear();
    auto view = registry.View<components::Position>();
    for (auto entity : view) {
        const auto& position = view.get<components::Position>(entity);
        Insert(entity, position);
    }
}

void GridHash::Insert(entt::entity entity, const components::Position& position) {
    const auto key = ToCellKey(position);
    cells_[key].push_back(CellEntry{entity, position});
}

void GridHash::QueryRadius(const components::Position& position, float radius, std::vector<entt::entity>& out) const {
    out.clear();
    if (radius <= 0.0f || cell_size_ <= 0.0f) {
        return;
    }

    const int min_x = static_cast<int>(std::floor((position.x - radius) / cell_size_));
    const int max_x = static_cast<int>(std::floor((position.x + radius) / cell_size_));
    const int min_y = static_cast<int>(std::floor((position.y - radius) / cell_size_));
    const int max_y = static_cast<int>(std::floor((position.y + radius) / cell_size_));

    const float radius_sq = radius * radius;
    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            GridCellKey key{x, y};
            auto it = cells_.find(key);
            if (it == cells_.end()) {
                continue;
            }
            for (const auto& entry : it->second) {
                const float dx = entry.position.x - position.x;
                const float dy = entry.position.y - position.y;
                if ((dx * dx + dy * dy) <= radius_sq) {
                    out.push_back(entry.entity);
                }
            }
        }
    }
}

float GridHash::CellSize() const {
    return cell_size_;
}

GridCellKey GridHash::ToCellKey(const components::Position& position) const {
    return GridCellKey{
        static_cast<int>(std::floor(position.x / cell_size_)),
        static_cast<int>(std::floor(position.y / cell_size_))
    };
}

} // namespace ant::spatial
