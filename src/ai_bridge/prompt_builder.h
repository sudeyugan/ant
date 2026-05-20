#pragma once

#include <cstdint>
#include <string>

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include "components/agent.h"
#include "components/social.h"
#include "components/transform.h"
#include "core/ecs_registry.h"

namespace ant::ai_bridge {

class PromptBuilder final {
public:
    static std::string BuildPrompt(const core::EcsRegistry& registry, entt::entity entity);
};

} // namespace ant::ai_bridge
