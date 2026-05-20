#include "view/debug_gui.h"

#include <imgui.h>

#include "core/ecs_registry.h"

namespace ant::view {

void DebugGui::Draw(const core::EcsRegistry& registry) const {
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }

    ImGui::Begin("World Debug");
    ImGui::Text("Entities: %zu", registry.EntityCount());
    ImGui::End();
}

} // namespace ant::view
