#include "ai_bridge/llm_client.h"

#include <string>
#include <utility>

#include "ai_bridge/async_task_queue.h"
#include "ai_bridge/prompt_builder.h"
#include "core/event_bus.h"
#include "core/events.h"

namespace ant::ai_bridge {

LlmClient::LlmClient(AsyncTaskQueue& queue) : queue_(queue) {}

void LlmClient::RequestDecision(const core::EcsRegistry& registry, entt::entity entity) const {
    std::string prompt = PromptBuilder::BuildPrompt(registry, entity);
    queue_.Enqueue([entity, prompt = std::move(prompt)]() {
        std::string decision = "idle";
        if (!prompt.empty()) {
            decision = "observe";
        }
        core::EventBus::Get().Publish<core::EventLLMDecisionReady>(entity, std::move(decision));
    });
}

} // namespace ant::ai_bridge
