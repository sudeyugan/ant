#pragma once

#include <cstdint>
#include <vector>

#include "components/transform.h"
#include "spatial/pathfinding.h"

namespace ant::spatial {

struct ResourceCell {
    float current{0.0f};
    float max{0.0f};
};

class ResourceGrid final {
public:
    ResourceGrid() = default;
    ResourceGrid(std::uint32_t width, std::uint32_t height, float max_sugar);

    void Resize(std::uint32_t width, std::uint32_t height, float max_sugar);
    bool InBounds(int x, int y) const;

    std::uint32_t Width() const;
    std::uint32_t Height() const;

    ResourceCell& At(int x, int y);
    const ResourceCell& At(int x, int y) const;

    void Growback(float rate);
    float Consume(int x, int y, float amount);

    GridPoint WorldToCell(const components::Position& position) const;
    components::Position CellToWorld(const GridPoint& cell) const;

    GridPoint FindBestCellInRadius(const GridPoint& origin, int radius) const;
    const GridMap& WalkableMap() const;

private:
    std::size_t Index(int x, int y) const;
    void RebuildWalkable();

    std::uint32_t width_{0};
    std::uint32_t height_{0};
    std::vector<ResourceCell> cells_{};
    GridMap walkable_{};
};

} // namespace ant::spatial
