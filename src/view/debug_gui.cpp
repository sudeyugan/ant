#include "view/debug_gui.h"

#include <algorithm>
#include <iterator>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "core/ecs_registry.h"
#include "core/event_bus.h"
#include "core/events.h"
#include "systems/history_sys.h"
#include "ai_bridge/llm_brain_manager.h"
#include "components/social.h"

namespace ant::view {

void DebugGui::Initialize(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    auto& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(14.0f, 12.0f);
    style.FramePadding = ImVec2(10.0f, 6.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;

    auto& colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.96f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.15f, 0.20f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.20f, 0.28f, 0.85f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.26f, 0.36f, 0.90f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.30f, 0.44f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.24f, 0.35f, 0.90f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.34f, 0.48f, 0.95f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.30f, 0.44f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.13f, 0.17f, 0.90f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.20f, 0.28f, 0.95f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.24f, 0.35f, 1.00f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ant::core::EventBus::Get().Subscribe<ant::core::EventChronicleReady>([this](const ant::core::EventChronicleReady& event) {
        chronicles_.push_back(event.report);
        if (chronicles_.size() > 20) {
            chronicles_.erase(chronicles_.begin());
        }
    });
}

void DebugGui::BeginFrame() const {
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void DebugGui::Draw(core::EcsRegistry& registry, const systems::WorldHistoryLog& history) const {
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }

    auto& reg = registry.Raw();

    // Count entities with various components
    int org_count = 0;
    int brain_count = 0;
    int family_count = 0;
    
    auto org_view = reg.view<ant::components::OrganizationComponent>();
    org_count = std::distance(org_view.begin(), org_view.end());
    
    auto brain_view = reg.view<ant::components::BrainComponent>();
    brain_count = std::distance(brain_view.begin(), brain_view.end());
    
    auto family_view = reg.view<ant::components::FamilyComponent>();
    family_count = std::distance(family_view.begin(), family_view.end());

    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float margin = 10.0f;
    const float top_bar_height = ImGui::GetFrameHeight() + margin * 1.5f;
    const float right_panel_width = std::min(400.0f, display.x * 0.32f);
    const float bottom_panel_height = std::min(260.0f, display.y * 0.32f);
    const float content_width = std::max(200.0f, display.x - right_panel_width - margin * 3.0f);
    const float sidebar_height =
        std::max(160.0f, display.y - top_bar_height - bottom_panel_height - margin * 3.0f);

    const ImGuiWindowFlags hud_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                       ImGuiWindowFlags_NoSavedSettings;
    const ImGuiWindowFlags panel_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    // Top HUD
    ImGui::SetNextWindowPos(ImVec2(margin, margin), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(display.x - margin * 2.0f, top_bar_height), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.85f);
    ImGui::Begin("HUD", nullptr, hud_flags);
    ImGui::Text("Entities: %zu", registry.EntityCount());
    ImGui::SameLine();
    ImGui::Text("Orgs: %d", org_count);
    ImGui::SameLine();
    ImGui::Text("Brains: %d", brain_count);
    ImGui::SameLine();
    ImGui::Text("Families: %d", family_count);
    ImGui::End();

    // Right Sidebar
    ImGui::SetNextWindowPos(ImVec2(display.x - margin - right_panel_width, margin * 2.0f + top_bar_height),
                            ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(right_panel_width, sidebar_height), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.96f);
    ImGui::Begin("Control Panel", nullptr, panel_flags);

    if (ImGui::BeginTabBar("SidebarTabs")) {
        if (ImGui::BeginTabItem("Overview")) {
            if (ImGui::BeginTable("StatsTable", 2, ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Entities");
                ImGui::TableNextColumn();
                ImGui::Text("%zu", registry.EntityCount());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Organizations");
                ImGui::TableNextColumn();
                ImGui::Text("%d", org_count);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Brain entities");
                ImGui::TableNextColumn();
                ImGui::Text("%d", brain_count);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Family units");
                ImGui::TableNextColumn();
                ImGui::Text("%d", family_count);
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Organizations")) {
            if (ImGui::BeginChild("OrgList", ImVec2(0, 0), true)) {
                for (auto org_entity : org_view) {
                    auto& org = reg.get<ant::components::OrganizationComponent>(org_entity);
                    ImGui::PushID(static_cast<int>(org.org_id));

                    if (ImGui::CollapsingHeader(("Org #" + std::to_string(org.org_id)).c_str(),
                                                ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Text("Leader: %u", static_cast<uint32_t>(org.leader));
                        ImGui::Text("Members: %zu", org.members.size());
                        ImGui::Text("Total wealth: %.1f", org.total_wealth);

                        if (ImGui::Button("Grant leader brain")) {
                            ant::ai_bridge::LLMBrainManager::Get().GrantIntelligence(registry, org.leader, true);
                        }
                    }

                    ImGui::PopID();
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Brains")) {
            ImGui::Text("Active brains: %d", brain_count);
            ImGui::Spacing();
            if (ImGui::Button("Grant Random Brain (+1)")) {
                ant::ai_bridge::LLMBrainManager::Get().GrantRandomIntelligence(registry, 1);
            }
            ImGui::SameLine();
            if (ImGui::Button("Grant Random Brain (+5)")) {
                ant::ai_bridge::LLMBrainManager::Get().GrantRandomIntelligence(registry, 5);
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    // Bottom Logs
    ImGui::SetNextWindowPos(ImVec2(margin, display.y - margin - bottom_panel_height), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(content_width, bottom_panel_height), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.96f);
    ImGui::Begin("Logs", nullptr, panel_flags);

    if (ImGui::BeginTabBar("LogTabs")) {
        if (ImGui::BeginTabItem("History")) {
            ImGui::Text("Total events: %zu", history.entries.size());
            ImGui::Separator();
            if (ImGui::BeginChild("HistoryScroll", ImVec2(0, 0), true)) {
                if (history.entries.empty()) {
                    ImGui::TextDisabled("No events yet.");
                } else {
                    for (int i = static_cast<int>(history.entries.size()) - 1;
                         i >= 0 && i >= static_cast<int>(history.entries.size()) - 50;
                         --i) {
                        ImGui::TextWrapped("%s", history.entries[i].c_str());
                    }
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Chronicles")) {
            if (ImGui::BeginChild("ChroniclesScroll", ImVec2(0, 0), true)) {
                if (chronicles_.empty()) {
                    ImGui::TextDisabled("No chronicle reports yet.");
                } else {
                    for (int i = static_cast<int>(chronicles_.size()) - 1; i >= 0; --i) {
                        ImGui::TextWrapped("%s", chronicles_[i].c_str());
                        ImGui::Separator();
                    }
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

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
