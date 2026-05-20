#include "llm_summary_client.h"

#include <sstream>
#include <utility>

#include "ai_bridge/async_task_queue.h"
#include "ai_bridge/deepseek_client.h"
#include "core/event_bus.h"
#include "core/events.h"

namespace ant::ai_bridge {

LLMSummaryClient::LLMSummaryClient(AsyncTaskQueue& queue, const std::string& api_key)
    : queue_(queue), api_key_(api_key) {}

void LLMSummaryClient::SetApiKey(const std::string& key) {
    api_key_ = key;
}

void LLMSummaryClient::Configure(const std::string& base_url, const std::string& api_key, const std::string& model) {
    base_url_ = base_url;
    api_key_ = api_key;
    model_ = model;
}

void LLMSummaryClient::RequestSummary(
    int alive_count,
    int starved_count,
    int orgs_formed,
    float avg_wealth,
    const std::string& key_events_text,
    uint64_t period_tick
) const {
    // Build prompt
    std::ostringstream prompt_ss;
    prompt_ss << "You are the chronicler of a virtual world. Here is the last period's data:\n\n";
    prompt_ss << "[Statistics]\n";
    prompt_ss << "- Alive entities: " << alive_count << "\n";
    prompt_ss << "- Starved this period: " << starved_count << " (primarily famine)\n";
    prompt_ss << "- Organizations formed: " << orgs_formed << "\n";
    prompt_ss << "- Average wealth: " << avg_wealth << "\n\n";
    prompt_ss << "[Key Events]\n" << key_events_text << "\n\n";
    prompt_ss << "Write a brief yearly chronicle (~200 words) highlighting conflicts, social evolution, ";
    prompt_ss << "and predictions for the next period.";

    std::string prompt = prompt_ss.str();
    const std::string base_url = base_url_;
    const std::string api_key = api_key_;
    const std::string model = model_;

    queue_.Enqueue([prompt = std::move(prompt), base_url, api_key, model, period_tick]() {
        std::string error;
        DeepSeekConfig config;
        config.base_url = base_url;
        config.api_key = api_key;
        config.model = model;
        config.temperature = 0.7f;
        config.max_tokens = 512;

        const std::string system_prompt = "You are the official chronicler of a virtual world.";
        auto result = RequestChatCompletion(config, system_prompt, prompt, error);
        if (!result) {
            const std::string report = "[DeepSeek error] " + error;
            ant::core::EventBus::Get().Publish<ant::core::EventChronicleReady>(report, period_tick);
            return;
        }

        ant::core::EventBus::Get().Publish<ant::core::EventChronicleReady>(*result, period_tick);
    });
}

}  // namespace ant::ai_bridge
