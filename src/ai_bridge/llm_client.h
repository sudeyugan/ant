#pragma once

#include <string>

#include <entt/entt.hpp>

namespace ant::core {
class EcsRegistry;
}

namespace ant::ai_bridge {

class AsyncTaskQueue;

class LlmClient final {
public:
    explicit LlmClient(AsyncTaskQueue& queue);

    void Configure(const std::string& base_url, const std::string& api_key, const std::string& model);

    void RequestDecision(const core::EcsRegistry& registry, entt::entity entity, float cooldown_seconds) const;

private:
    AsyncTaskQueue& queue_;
    std::string base_url_{"https://api.deepseek.com"};
    std::string api_key_{};
    std::string model_{"deepseek-v4-pro"};
};

} // namespace ant::ai_bridge
