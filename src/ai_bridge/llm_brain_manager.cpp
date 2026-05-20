#include "llm_brain_manager.h"

#include <algorithm>
#include <random>
#include <vector>

#include "core/ecs_registry.h"
#include "core/event_bus.h"
#include "core/events.h"
#include "components/agent.h"
#include "components/social.h"
#include "components/transform.h"

namespace ant::ai_bridge {

LLMBrainManager::LLMBrainManager() = default;

LLMBrainManager& LLMBrainManager::Get() {
    static LLMBrainManager instance;
    return instance;
}

void LLMBrainManager::GrantIntelligence(ant::core::EcsRegistry& registry, entt::entity entity, bool is_leader) {
    auto& reg = registry.Raw();
    
    if (!reg.valid(entity)) return;

    uint64_t llm_id = next_llm_id_++;
    reg.emplace_or_replace<ant::components::BrainComponent>(entity, llm_id, 0.0f, is_leader);

    if (!reg.any_of<ant::components::MemoryRef>(entity)) {
        reg.emplace<ant::components::MemoryRef>(entity, static_cast<std::uint32_t>(llm_id));
    }
    
    ant::core::EventBus::Get().Publish<ant::core::EventBrainGranted>(entity, llm_id);
}

void LLMBrainManager::RevokeIntelligence(ant::core::EcsRegistry& registry, entt::entity entity) {
    auto& reg = registry.Raw();
    
    if (!reg.valid(entity) || !reg.any_of<ant::components::BrainComponent>(entity)) return;

    reg.remove<ant::components::BrainComponent>(entity);
    ant::core::EventBus::Get().Publish<ant::core::EventBrainRevoked>(entity);
}

void LLMBrainManager::GrantRandomIntelligence(ant::core::EcsRegistry& registry, size_t count) {
    auto& reg = registry.Raw();
    
    // Collect all non-brain entities
    std::vector<entt::entity> candidates;
    auto& storage = reg.storage<entt::entity>();
    for (auto e : storage) {
        if (!reg.any_of<ant::components::Position>(e)) {
            continue;
        }
        if (!reg.any_of<ant::components::BrainComponent>(e)) {
            candidates.push_back(e);
        }
    }

    // Grant to random subset
    if (candidates.empty()) return;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(candidates.begin(), candidates.end(), gen);
    
    size_t grant_count = std::min(count, candidates.size());
    for (size_t i = 0; i < grant_count; ++i) {
        GrantIntelligence(registry, candidates[i], false);
    }
}

bool LLMBrainManager::HasBrain(const ant::core::EcsRegistry& registry, entt::entity entity) const {
    auto& reg = registry.Raw();
    return reg.any_of<ant::components::BrainComponent>(entity);
}

}  // namespace ant::ai_bridge
