#include <cstdint>

#include <entt/entt.hpp>
#include <vector>

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

    constexpr std::uint32_t entity_count = 256;
    std::vector<ant::core::Entity> entities;
    entities.reserve(entity_count);
    ant::core::Entity first_entity = entt::null;
    for (std::uint32_t i = 0; i < entity_count; ++i) {
        auto entity = registry.CreateEntity();
        entities.push_back(entity);
        if (i == 0) {
            first_entity = entity;
        }
        registry.AddComponent<ant::components::Position>(entity, ant::components::Position{static_cast<float>(i), 0.0f});
        registry.AddComponent<ant::components::Velocity>(entity, ant::components::Velocity{0.5f, 0.0f});
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

    for (int frame = 0; frame < 600; ++frame) {
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

        renderer.BeginFrame();
        renderer.DrawEntities(registry);
        debug_gui.Draw(registry);
        renderer.EndFrame();
    }

    task_queue.Stop();
    transient_pool.Reset();
    return 0;
}
