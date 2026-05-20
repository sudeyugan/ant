#pragma once

#include <cstdint>
#include <string>

namespace ant::ai_bridge {

class AsyncTaskQueue;

class LLMSummaryClient {
public:
    explicit LLMSummaryClient(AsyncTaskQueue& queue, const std::string& api_key = "");

    void Configure(const std::string& base_url, const std::string& api_key, const std::string& model);

    void RequestSummary(
        int alive_count,
        int starved_count,
        int orgs_formed,
        float avg_wealth,
        const std::string& key_events_text,
        uint64_t period_tick
    ) const;

    void SetApiKey(const std::string& key);

private:
    AsyncTaskQueue& queue_;
    std::string base_url_{"https://api.deepseek.com"};
    std::string api_key_{};
    std::string model_{"deepseek-v4-pro"};
};

}  // namespace ant::ai_bridge
