#pragma once

#include <mutex>
#include <utility>

#include <entt/entt.hpp>

namespace ant::core {

class EventBus final {
public:
    static EventBus& Get();

    template <typename Event, typename... Args>
    void Publish(Args&&... args) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        dispatcher_.enqueue<Event>(std::forward<Args>(args)...);
    }

    template <typename Event, typename Function>
    void Subscribe(Function&& function) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        dispatcher_.sink<Event>().connect(std::forward<Function>(function));
    }

    template <typename Event, auto Candidate, typename Type>
    void Subscribe(Type& instance) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        dispatcher_.sink<Event>().template connect<Candidate>(instance);
    }

    void Update();

private:
    EventBus() = default;

    std::recursive_mutex mutex_{};
    entt::dispatcher dispatcher_{};
};

} // namespace ant::core
