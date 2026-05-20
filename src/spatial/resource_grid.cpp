#include "spatial/resource_grid.h"

#include <algorithm>

namespace ant::spatial {

ResourceGrid::ResourceGrid(std::uint32_t width, std::uint32_t height, float max_sugar) {
    Resize(width, height, max_sugar);
}

void ResourceGrid::Resize(std::uint32_t width, std::uint32_t height, float max_sugar) {
    width_ = width;
    height_ = height;
    cells_.assign(static_cast<std::size_t>(width_) * height_, ResourceCell{max_sugar, max_sugar});
    RebuildWalkable();
}

bool ResourceGrid::InBounds(int x, int y) const {
    return x >= 0 && y >= 0 && static_cast<std::uint32_t>(x) < width_ && static_cast<std::uint32_t>(y) < height_;
}

std::uint32_t ResourceGrid::Width() const {
    return width_;
}

std::uint32_t ResourceGrid::Height() const {
    return height_;
}

ResourceCell& ResourceGrid::At(int x, int y) {
    return cells_[Index(x, y)];
}

const ResourceCell& ResourceGrid::At(int x, int y) const {
    return cells_[Index(x, y)];
}

void ResourceGrid::Growback(float rate) {
    if (rate <= 0.0f) {
        return;
    }
    for (auto& cell : cells_) {
        cell.current = std::min(cell.current + rate, cell.max);
    }
}

float ResourceGrid::Consume(int x, int y, float amount) {
    if (!InBounds(x, y) || amount <= 0.0f) {
        return 0.0f;
    }
    auto& cell = At(x, y);
    const float taken = std::min(cell.current, amount);
    cell.current -= taken;
    return taken;
}

GridPoint ResourceGrid::WorldToCell(const components::Position& position) const {
    return GridPoint{static_cast<int>(position.x), static_cast<int>(position.y)};
}

components::Position ResourceGrid::CellToWorld(const GridPoint& cell) const {
    return components::Position{static_cast<float>(cell.x) + 0.5f, static_cast<float>(cell.y) + 0.5f};
}

GridPoint ResourceGrid::FindBestCellInRadius(const GridPoint& origin, int radius) const {
    if (radius <= 0) {
        return origin;
    }

    float best_sugar = -1.0f;
    GridPoint best_cell = origin;
    const int min_x = origin.x - radius;
    const int max_x = origin.x + radius;
    const int min_y = origin.y - radius;
    const int max_y = origin.y + radius;

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            if (!InBounds(x, y)) {
                continue;
            }
            const auto& cell = At(x, y);
            if (cell.current > best_sugar) {
                best_sugar = cell.current;
                best_cell = GridPoint{x, y};
            }
        }
    }

    return best_cell;
}

const GridMap& ResourceGrid::WalkableMap() const {
    return walkable_;
}

std::size_t ResourceGrid::Index(int x, int y) const {
    return static_cast<std::size_t>(y) * width_ + static_cast<std::size_t>(x);
}

void ResourceGrid::RebuildWalkable() {
    walkable_.width = width_;
    walkable_.height = height_;
    walkable_.walkable.assign(static_cast<std::size_t>(width_) * height_, 1);
}

} // namespace ant::spatial
