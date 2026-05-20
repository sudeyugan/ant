#include "view/renderer_2d.h"

#include "core/ecs_registry.h"

namespace ant::view {

void Renderer2D::BeginFrame() const {}

void Renderer2D::DrawEntities(const core::EcsRegistry&) const {}

void Renderer2D::EndFrame() const {}

} // namespace ant::view
