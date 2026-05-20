#pragma once

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>

#include <entt/entt.hpp>

#include "components/transform.h"

namespace ant::core {
class EcsRegistry;
}

namespace ant::spatial {

struct GridCellKey {
    int x{0};
    int y{0};

    bool operator==(const GridCellKey& other) const {
        return x == other.x && y == other.y;
    }
};

struct GridCellKeyHash {
    std::size_t operator()(const GridCellKey& key) const noexcept {
        const std::size_t h1 = std::hash<int>{}(key.x);
        const std::size_t h2 = std::hash<int>{}(key.y);
        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
    }
};

class GridHash final {
public:
    explicit GridHash(float cell_size = 1.0f);

    void Clear();
    void Rebuild(const core::EcsRegistry& registry);
    void Insert(entt::entity entity, const components::Position& position);
    void QueryRadius(const components::Position& position, float radius, std::vector<entt::entity>& out) const;

    float CellSize() const;

private:
    GridCellKey ToCellKey(const components::Position& position) const;

    float cell_size_{1.0f};
    struct CellEntry {
        entt::entity entity{};
        components::Position position{};
    };

    std::unordered_map<GridCellKey, std::vector<CellEntry>, GridCellKeyHash> cells_{};
};

} // namespace ant::spatial
