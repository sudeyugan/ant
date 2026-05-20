#pragma once

#include <cstdint>
#include <vector>

#include <entt/entity/entity.hpp>

namespace ant::core {
    class EcsRegistry;
}

namespace ant::ai_bridge {

class LLMBrainManager {
public:
    static LLMBrainManager& Get();

    void GrantIntelligence(ant::core::EcsRegistry& registry, entt::entity entity, bool is_leader = false);
    void RevokeIntelligence(ant::core::EcsRegistry& registry, entt::entity entity);
    void GrantRandomIntelligence(ant::core::EcsRegistry& registry, size_t count);

    bool HasBrain(const ant::core::EcsRegistry& registry, entt::entity entity) const;
    
private:
    LLMBrainManager();
    uint64_t next_llm_id_{1};
};

}  // namespace ant::ai_bridge
