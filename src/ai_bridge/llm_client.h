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

    void RequestDecision(const core::EcsRegistry& registry, entt::entity entity) const;

private:
    AsyncTaskQueue& queue_;
};

} // namespace ant::ai_bridge
