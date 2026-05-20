#include <cstdint>

#include <algorithm>
#include <entt/entt.hpp>
#include <vector>

#include <GLFW/glfw3.h>

#include "ai_bridge/async_task_queue.h"
#include "ai_bridge/llm_client.h"
#include "ai_bridge/llm_brain_manager.h"
#include "ai_bridge/llm_summary_client.h"
#include "components/agent.h"
#include "components/social.h"
#include "components/transform.h"
#include "core/ecs_registry.h"
#include "core/event_bus.h"
#include "core/memory_pool.h"
#include "core/simulation_config.h"
#include "core/timer.h"
#include "spatial/grid_hash.h"
#include "spatial/resource_grid.h"
#include "systems/history_sys.h"
#include "systems/movement_sys.h"
#include "systems/sugar_growback_sys.h"
#include "systems/sugarscape_sys.h"
#include "systems/survival_sys.h"
#include "systems/emergence_sys.h"
#include "systems/chronicle_sys.h"
#include "view/debug_gui.h"
#include "view/renderer_2d.h"

int main() {
    if (!glfwInit()) {
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Society Simulator", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    const auto config = ant::core::LoadSimulationConfig("e:\\ant\\config\\simulation.json");

    ant::core::EcsRegistry registry;
    ant::core::MemoryPool transient_pool(1 << 20);
    ant::core::Timer timer(1.0 / 60.0);
    ant::spatial::GridHash spatial_hash(1.0f);
    ant::spatial::ResourceGrid resource_grid(config.map_width, config.map_height, config.max_sugar);

    ant::systems::MovementSystem movement;
    ant::systems::SurvivalSystem survival;
    ant::systems::SugarscapeSystem sugarscape;
    ant::systems::SugarGrowbackSystem growback;
    ant::systems::EmergenceSystem emergence;
    ant::systems::ChronicleSystem chronicle;
    ant::systems::WorldHistoryLog history_log;
    ant::systems::HistorySystem::Register(history_log);

    ant::ai_bridge::AsyncTaskQueue task_queue;
    task_queue.Start(1);
    ant::ai_bridge::LlmClient llm(task_queue);
    ant::ai_bridge::LLMSummaryClient summary_client(task_queue);
    llm.Configure(config.deepseek_base_url, config.deepseek_api_key, config.deepseek_model);
    summary_client.Configure(config.deepseek_base_url, config.deepseek_api_key, config.deepseek_model);

    ant::view::Renderer2D renderer;
    ant::view::DebugGui debug_gui;
    debug_gui.Initialize(window);
    ant::view::RenderSettings render_settings;

    constexpr std::uint32_t entity_count = 256;
    std::vector<ant::core::Entity> entities;
    entities.reserve(entity_count);
    ant::core::Entity first_entity = entt::null;
    const std::uint32_t columns = config.map_width > 0 ? config.map_width : 1u;
    for (std::uint32_t i = 0; i < entity_count; ++i) {
        auto entity = registry.CreateEntity();
        entities.push_back(entity);
        if (i == 0) {
            first_entity = entity;
        }
        const float x = static_cast<float>(i % columns);
        const float y = static_cast<float>((i / columns) % (config.map_height > 0 ? config.map_height : 1u));
        registry.AddComponent<ant::components::Position>(entity, ant::components::Position{x, y});
        registry.AddComponent<ant::components::Velocity>(entity, ant::components::Velocity{0.0f, 0.0f});
        registry.AddComponent<ant::components::Health>(entity, ant::components::Health{100.0f, 100.0f});
        registry.AddComponent<ant::components::Hunger>(
            entity, ant::components::Hunger{100.0f, config.metabolism_rate, 100.0f});
        registry.AddComponent<ant::components::Vision>(entity, ant::components::Vision{config.vision});
        registry.AddComponent<ant::components::Wealth>(entity, ant::components::Wealth{10.0f, 5.0f});
    }

    // Create ECS singleton for emergence state
    auto emergence_state_entity = registry.CreateEntity();
    registry.AddComponent<ant::components::EmergenceState>(emergence_state_entity, ant::components::EmergenceState{});

    // Grant brain to a few random entities (future leaders)
    ant::ai_bridge::LLMBrainManager::Get().GrantRandomIntelligence(registry, 3);

    bool is_paused = !config.auto_step;
    bool prev_pause_key = false;
    bool prev_step_key = false;
    std::uint64_t tick = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        const bool pause_key = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
        if (pause_key && !prev_pause_key) {
            is_paused = !is_paused;
            if (!is_paused) {
                timer.Reset();
            }
        }
        prev_pause_key = pause_key;

        bool step_requested = false;
        const bool step_key = glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS;
        if (step_key && !prev_step_key) {
            step_requested = true;
        }
        prev_step_key = step_key;

        if (!is_paused) {
            timer.Tick();
        } else if (step_requested) {
            timer.ForceStep();
        }

        while (timer.ShouldStep()) {
            const float dt = static_cast<float>(timer.FixedStepSeconds());
            spatial_hash.Rebuild(registry);
            
            // Execute systems in order
            sugarscape.Update(registry, resource_grid, spatial_hash, config, dt);
            movement.Update(registry, dt);
            survival.Update(registry, dt);
            emergence.Update(registry, spatial_hash);
            growback.Update(resource_grid, config, tick);

            // LLM decision dispatch with cooldown
            {
                auto& reg = registry.Raw();
                auto brain_view = reg.view<ant::components::BrainComponent>();
                for (auto e : brain_view) {
                    auto& brain = reg.get<ant::components::BrainComponent>(e);
                    brain.cooldown_remaining = std::max(0.0f, brain.cooldown_remaining - dt);
                    if (brain.cooldown_remaining <= 0.0f) {
                        brain.cooldown_remaining = config.llm_decision_cooldown;
                        llm.RequestDecision(registry, e, config.llm_decision_cooldown);
                    }
                }
            }

            // Process all events
            ant::core::EventBus::Get().Update();

            chronicle.Update(registry, config, history_log, summary_client, tick);
            
            timer.ConsumeStep();
            ++tick;
        }

        int framebuffer_width = 0;
        int framebuffer_height = 0;
        glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

        const float world_width = static_cast<float>(config.map_width > 0 ? config.map_width : 1u);
        const float world_height = static_cast<float>(config.map_height > 0 ? config.map_height : 1u);
        renderer.BeginFrame(framebuffer_width, framebuffer_height, world_width, world_height);
        renderer.DrawWorld(resource_grid, registry, render_settings);
        debug_gui.BeginFrame();
        debug_gui.Draw(registry, history_log);
        debug_gui.EndFrame();
        renderer.EndFrame();
        glfwSwapBuffers(window);
    }

    debug_gui.Shutdown();
    task_queue.Stop();
    transient_pool.Reset();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
