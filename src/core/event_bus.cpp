#include "core/event_bus.h"

namespace ant::core {

EventBus& EventBus::Get() {
    static EventBus instance;
    return instance;
}

void EventBus::Update() {
    std::vector<void (*)()> drainers;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        drainers = drainers_;
    }
    for (auto drain : drainers) {
        if (drain) {
            drain();
        }
    }
}

} // namespace ant::core
