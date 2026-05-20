#pragma once

#include <cstdint>
#include <vector>

namespace ant::spatial {

struct GridPoint {
    int x{0};
    int y{0};
};

struct GridMap {
    std::uint32_t width{0};
    std::uint32_t height{0};
    std::vector<std::uint8_t> walkable{};

    bool IsWalkable(int x, int y) const;
};

class Pathfinding final {
public:
    static std::vector<GridPoint> AStar(const GridMap& map, GridPoint start, GridPoint goal);
};

} // namespace ant::spatial
