#include "view/renderer_2d.h"

#include <GLFW/glfw3.h>

#include "components/transform.h"
#include "core/ecs_registry.h"

namespace ant::view {

void Renderer2D::BeginFrame(std::int32_t width, std::int32_t height) const {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(width), 0.0, static_cast<double>(height), -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(4.0f);
}

void Renderer2D::DrawEntities(const core::EcsRegistry& registry) const {
    auto view = registry.View<components::Position>();
    glBegin(GL_POINTS);
    glColor3f(0.2f, 0.9f, 0.2f);
    for (auto entity : view) {
        const auto& position = view.get<components::Position>(entity);
        glVertex2f(position.x, position.y);
    }
    glEnd();
}

void Renderer2D::EndFrame() const {}

} // namespace ant::view
