#include <cstdint>

#include <entt/entt.hpp>
#include <vector>

#include <GLFW/glfw3.h>

#include "ai_bridge/async_task_queue.h"
#include "ai_bridge/llm_client.h"
#include "components/agent.h"
#include "components/social.h"
#include "components/transform.h"
#include "core/ecs_registry.h"
#include "core/event_bus.h"
#include "core/memory_pool.h"
#include "core/timer.h"
#include "spatial/grid_hash.h"
#include "systems/history_sys.h"
#include "systems/movement_sys.h"
#include "systems/social_sys.h"
#include "systems/survival_sys.h"
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

    ant::core::EcsRegistry registry;
    ant::core::MemoryPool transient_pool(1 << 20);
    ant::core::Timer timer(1.0 / 60.0);
    ant::spatial::GridHash spatial_hash(1.0f);

    ant::systems::MovementSystem movement;
    ant::systems::SurvivalSystem survival;
    ant::systems::SocialSystem social;
    ant::systems::WorldHistoryLog history_log;
    ant::systems::HistorySystem::Register(history_log);

    ant::ai_bridge::AsyncTaskQueue task_queue;
    task_queue.Start(1);
    ant::ai_bridge::LlmClient llm(task_queue);

    ant::view::Renderer2D renderer;
    ant::view::DebugGui debug_gui;
    debug_gui.Initialize(window);

    constexpr std::uint32_t entity_count = 256;
    std::vector<ant::core::Entity> entities;
    entities.reserve(entity_count);
    ant::core::Entity first_entity = entt::null;
    constexpr float spacing = 16.0f;
    constexpr std::uint32_t columns = 64;
    constexpr float origin_x = 32.0f;
    constexpr float origin_y = 32.0f;
    for (std::uint32_t i = 0; i < entity_count; ++i) {
        auto entity = registry.CreateEntity();
        entities.push_back(entity);
        if (i == 0) {
            first_entity = entity;
        }
        const float x = origin_x + static_cast<float>(i % columns) * spacing;
        const float y = origin_y + static_cast<float>(i / columns) * spacing;
        registry.AddComponent<ant::components::Position>(entity, ant::components::Position{x, y});
        registry.AddComponent<ant::components::Velocity>(entity, ant::components::Velocity{0.5f, 0.2f});
        registry.AddComponent<ant::components::Health>(entity, ant::components::Health{100.0f, 100.0f});
        registry.AddComponent<ant::components::Hunger>(entity, ant::components::Hunger{100.0f, 0.2f});
        registry.AddComponent<ant::components::Faction>(entity, ant::components::Faction{i % 4});
        registry.AddComponent<ant::components::Wealth>(entity, ant::components::Wealth{10.0f, 5.0f});
        registry.AddComponent<ant::components::MemoryRef>(entity, ant::components::MemoryRef{i});
        registry.AddComponent<ant::components::SocialConnections>(entity, ant::components::SocialConnections{});
    }

    if (registry.Raw().valid(first_entity)) {
        auto& connections = registry.GetComponent<ant::components::SocialConnections>(first_entity);
        for (std::uint32_t i = 1; i < 5 && i < entity_count; ++i) {
            connections.enemies.push_back(entities[i]);
        }
        llm.RequestDecision(registry, first_entity);
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        timer.Tick();
        while (timer.ShouldStep()) {
            const float dt = static_cast<float>(timer.FixedStepSeconds());
            movement.Update(registry, dt);
            survival.Update(registry, dt);
            social.Update(registry, dt);
            spatial_hash.Rebuild(registry);
            ant::core::EventBus::Get().Update();
            timer.ConsumeStep();
        }

        int framebuffer_width = 0;
        int framebuffer_height = 0;
        glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

        renderer.BeginFrame(framebuffer_width, framebuffer_height);
        renderer.DrawEntities(registry);
        debug_gui.BeginFrame();
        debug_gui.Draw(registry);
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
