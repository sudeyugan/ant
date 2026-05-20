#include "spatial/pathfinding.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace ant::spatial {

namespace {

struct Node {
    int x{0};
    int y{0};
    float g{0.0f};
    float f{0.0f};
    int parent{-1};
};

std::size_t Index(int x, int y, std::uint32_t width) {
    return static_cast<std::size_t>(y) * width + static_cast<std::size_t>(x);
}

float Heuristic(const GridPoint& a, const GridPoint& b) {
    return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

} // namespace

bool GridMap::IsWalkable(int x, int y) const {
    if (x < 0 || y < 0 || static_cast<std::uint32_t>(x) >= width || static_cast<std::uint32_t>(y) >= height) {
        return false;
    }
    const auto idx = Index(x, y, width);
    if (idx >= walkable.size()) {
        return false;
    }
    return walkable[idx] != 0;
}

std::vector<GridPoint> Pathfinding::AStar(const GridMap& map, GridPoint start, GridPoint goal) {
    std::vector<GridPoint> path{};
    if (!map.IsWalkable(start.x, start.y) || !map.IsWalkable(goal.x, goal.y)) {
        return path;
    }

    const std::size_t total = static_cast<std::size_t>(map.width) * map.height;
    std::vector<Node> nodes(total);
    std::vector<std::uint8_t> open_set(total, 0);
    std::vector<std::uint8_t> closed_set(total, 0);

    auto cmp = [&](int lhs, int rhs) { return nodes[lhs].f > nodes[rhs].f; };
    std::priority_queue<int, std::vector<int>, decltype(cmp)> open_queue(cmp);

    const auto start_index = static_cast<int>(Index(start.x, start.y, map.width));
    nodes[start_index] = {start.x, start.y, 0.0f, Heuristic(start, goal), -1};
    open_set[start_index] = 1;
    open_queue.push(start_index);

    const int offsets[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    while (!open_queue.empty()) {
        const int current_index = open_queue.top();
        open_queue.pop();

        if (closed_set[current_index]) {
            continue;
        }

        const auto& current = nodes[current_index];
        if (current.x == goal.x && current.y == goal.y) {
            int node_index = current_index;
            while (node_index >= 0) {
                const auto& node = nodes[node_index];
                path.push_back(GridPoint{node.x, node.y});
                node_index = node.parent;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        closed_set[current_index] = 1;

        for (const auto& offset : offsets) {
            const int nx = current.x + offset[0];
            const int ny = current.y + offset[1];
            if (!map.IsWalkable(nx, ny)) {
                continue;
            }

            const int neighbor_index = static_cast<int>(Index(nx, ny, map.width));
            if (closed_set[neighbor_index]) {
                continue;
            }

            const float tentative_g = current.g + 1.0f;
            if (!open_set[neighbor_index] || tentative_g < nodes[neighbor_index].g) {
                nodes[neighbor_index].x = nx;
                nodes[neighbor_index].y = ny;
                nodes[neighbor_index].g = tentative_g;
                nodes[neighbor_index].f = tentative_g + Heuristic(GridPoint{nx, ny}, goal);
                nodes[neighbor_index].parent = current_index;
                open_set[neighbor_index] = 1;
                open_queue.push(neighbor_index);
            }
        }
    }

    return path;
}

} // namespace ant::spatial
