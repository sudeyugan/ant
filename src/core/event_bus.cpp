#include "core/event_bus.h"

namespace ant::core {

EventBus& EventBus::Get() {
    static EventBus instance;
    return instance;
}

void EventBus::Update() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    dispatcher_.update();
}

} // namespace ant::core
