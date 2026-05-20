#pragma once

#include <functional>
#include <mutex>
#include <typeindex>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ant::core {

class EventBus final {
public:
    static EventBus& Get();

    template <typename Event, typename... Args>
    void Publish(Args&&... args) {
        auto& channel = GetChannel<Event>();
        {
            std::lock_guard<std::mutex> lock(channel.mutex);
            Event event{std::forward<Args>(args)...};
            channel.queue.push_back(std::move(event));
        }
    }

    template <typename Event, typename Function>
    void Subscribe(Function&& function) {
        auto& channel = GetChannel<Event>();
        {
            std::lock_guard<std::mutex> lock(channel.mutex);
            channel.subscribers.emplace_back(std::forward<Function>(function));
        }
    }

    void Update();

private:
    EventBus() = default;

    template <typename Event>
    struct Channel {
        std::mutex mutex{};
        std::vector<Event> queue{};
        std::vector<std::function<void(const Event&)>> subscribers{};
    };

    template <typename Event>
    Channel<Event>& GetChannel() {
        Register<Event>();
        static Channel<Event> channel;
        return channel;
    }

    template <typename Event>
    static Channel<Event>& GetChannelNoRegister() {
        static Channel<Event> channel;
        return channel;
    }

    template <typename Event>
    void Register() {
        std::lock_guard<std::mutex> lock(mutex_);
        const std::type_index id(typeid(Event));
        if (registered_.insert(id).second) {
            drainers_.push_back(&EventBus::Drain<Event>);
        }
    }

    template <typename Event>
    static void Drain() {
        auto& channel = GetChannelNoRegister<Event>();
        std::vector<Event> events;
        std::vector<std::function<void(const Event&)>> subscribers;
        {
            std::lock_guard<std::mutex> lock(channel.mutex);
            events.swap(channel.queue);
            subscribers = channel.subscribers;
        }
        for (const auto& event : events) {
            for (const auto& subscriber : subscribers) {
                subscriber(event);
            }
        }
    }

    std::mutex mutex_{};
    std::unordered_set<std::type_index> registered_{};
    std::vector<void (*)()> drainers_{};
};

} // namespace ant::core
