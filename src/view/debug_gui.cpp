#include "view/debug_gui.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "core/ecs_registry.h"

namespace ant::view {

void DebugGui::Initialize(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void DebugGui::BeginFrame() const {
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void DebugGui::Draw(const core::EcsRegistry& registry) const {
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }

    ImGui::Begin("World Debug");
    ImGui::Text("Entities: %zu", registry.EntityCount());
    ImGui::End();
}

void DebugGui::EndFrame() const {
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugGui::Shutdown() const {
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

} // namespace ant::view
