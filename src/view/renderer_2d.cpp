#include "view/renderer_2d.h"

#include <algorithm>

#include <GLFW/glfw3.h>

#include "components/agent.h"
#include "components/transform.h"
#include "core/ecs_registry.h"
#include "spatial/resource_grid.h"

namespace ant::view {

namespace {

constexpr float kBaseEntitySize = 0.25f;
constexpr float kEntitySizeScale = 0.55f;

float Clamp01(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

void DrawResourceGrid(const spatial::ResourceGrid& grid) {
    glBegin(GL_QUADS);
    for (std::uint32_t y = 0; y < grid.Height(); ++y) {
        for (std::uint32_t x = 0; x < grid.Width(); ++x) {
            const auto& cell = grid.At(static_cast<int>(x), static_cast<int>(y));
            const float ratio = (cell.max > 0.0f) ? Clamp01(cell.current / cell.max) : 0.0f;
            const float r = 0.08f + 0.82f * ratio;
            const float g = 0.07f + 0.78f * ratio;
            const float b = 0.05f + 0.15f * ratio;
            glColor3f(r, g, b);
            const float fx = static_cast<float>(x);
            const float fy = static_cast<float>(y);
            glVertex2f(fx, fy);
            glVertex2f(fx + 1.0f, fy);
            glVertex2f(fx + 1.0f, fy + 1.0f);
            glVertex2f(fx, fy + 1.0f);
        }
    }
    glEnd();
}

void DrawEntities(const core::EcsRegistry& registry) {
    auto view = registry.View<components::Position, components::Hunger>();
    glBegin(GL_QUADS);
    for (auto entity : view) {
        const auto& position = view.get<components::Position>(entity);
        const auto& hunger = view.get<components::Hunger>(entity);
        const float ratio = (hunger.max > 0.0f) ? Clamp01(hunger.value / hunger.max) : 0.0f;
        const float size = kBaseEntitySize + kEntitySizeScale * ratio;
        const float r = 1.0f - ratio;
        const float g = 0.2f + 0.8f * ratio;
        const float b = 0.15f;
        glColor3f(r, g, b);

        const float x = position.x;
        const float y = position.y;
        glVertex2f(x - size, y - size);
        glVertex2f(x + size, y - size);
        glVertex2f(x + size, y + size);
        glVertex2f(x - size, y + size);
    }
    glEnd();
}

void DrawVisionAndTargets(const core::EcsRegistry& registry,
                          const spatial::ResourceGrid& grid,
                          std::uint32_t max_entities,
                          bool draw_targets) {
    auto view = registry.View<components::Position, components::Vision>();
    std::uint32_t count = 0;

    glColor4f(0.2f, 0.7f, 1.0f, 0.6f);
    for (auto entity : view) {
        if (count++ >= max_entities) {
            break;
        }
        const auto& position = view.get<components::Position>(entity);
        const auto& vision = view.get<components::Vision>(entity);
        const float r = static_cast<float>(vision.range) + 0.5f;

        glBegin(GL_LINE_LOOP);
        glVertex2f(position.x - r, position.y - r);
        glVertex2f(position.x + r, position.y - r);
        glVertex2f(position.x + r, position.y + r);
        glVertex2f(position.x - r, position.y + r);
        glEnd();

        if (draw_targets) {
            const auto cell = grid.WorldToCell(position);
            const auto target = grid.FindBestCellInRadius(cell, vision.range);
            if (target.x != cell.x || target.y != cell.y) {
                const auto target_world = grid.CellToWorld(target);
                glBegin(GL_LINES);
                glVertex2f(position.x, position.y);
                glVertex2f(target_world.x, target_world.y);
                glEnd();
            }
        }
    }
}

} // namespace

void Renderer2D::BeginFrame(std::int32_t framebuffer_width, std::int32_t framebuffer_height,
                            float world_width, float world_height) const {
    glViewport(0, 0, framebuffer_width, framebuffer_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(world_width), 0.0, static_cast<double>(world_height), -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer2D::DrawWorld(const spatial::ResourceGrid& grid,
                           const core::EcsRegistry& registry,
                           const RenderSettings& settings) const {
    if (settings.draw_sugar) {
        DrawResourceGrid(grid);
    }
    if (settings.draw_entities) {
        DrawEntities(registry);
    }
    if (settings.draw_vision) {
        DrawVisionAndTargets(registry, grid, settings.max_debug_entities, settings.draw_targets);
    }
}

void Renderer2D::EndFrame() const {}

} // namespace ant::view
