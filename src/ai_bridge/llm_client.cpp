#include "ai_bridge/llm_client.h"

#include <string>
#include <utility>

#include "ai_bridge/async_task_queue.h"
#include "ai_bridge/deepseek_client.h"
#include "ai_bridge/prompt_builder.h"
#include "core/event_bus.h"
#include "core/events.h"
#include "core/ecs_registry.h"
#include "components/social.h"

namespace ant::ai_bridge {

LlmClient::LlmClient(AsyncTaskQueue& queue) : queue_(queue) {}

void LlmClient::Configure(const std::string& base_url, const std::string& api_key, const std::string& model) {
    base_url_ = base_url;
    api_key_ = api_key;
    model_ = model;
}

void LlmClient::RequestDecision(const core::EcsRegistry& registry, entt::entity entity, float cooldown_seconds) const {
    auto& reg = registry.Raw();
    
    // Only request decision for entities with BrainComponent
    if (!reg.any_of<components::BrainComponent>(entity)) {
        return;
    }

    const auto& brain = reg.get<components::BrainComponent>(entity);
    
    // Check cooldown
    if (brain.cooldown_remaining > 0.0f) {
        return;
    }
    (void)cooldown_seconds;

    std::string prompt = PromptBuilder::BuildPrompt(registry, entity);
    const std::string base_url = base_url_;
    const std::string api_key = api_key_;
    const std::string model = model_;

    queue_.Enqueue([entity, prompt = std::move(prompt), base_url, api_key, model]() {
        std::string error;
        DeepSeekConfig config;
        config.base_url = base_url;
        config.api_key = api_key;
        config.model = model;
        config.temperature = 0.3f;
        config.max_tokens = 128;

        const std::string system_prompt =
            "You are a decision engine for one simulated entity. "
            "Return a single short action keyword such as: observe, move, trade, rest.";

        auto result = RequestChatCompletion(config, system_prompt, prompt, error);
        if (!result) {
            const std::string decision = "[LLM error] " + error;
            core::EventBus::Get().Publish<core::EventLLMDecisionReady>(entity, decision);
            return;
        }

        core::EventBus::Get().Publish<core::EventLLMDecisionReady>(entity, *result);
    });
}

} // namespace ant::ai_bridge
