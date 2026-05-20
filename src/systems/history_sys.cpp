#include "systems/history_sys.h"

#include <cstdint>
#include <string>

#include "core/event_bus.h"
#include "core/events.h"

namespace ant::systems {

void HistorySystem::Register(WorldHistoryLog& log) {
    core::EventBus::Get().Subscribe<core::EventEntityStarved>([&log](const core::EventEntityStarved& event) {
        log.entries.emplace_back("Entity " + std::to_string(static_cast<std::uint32_t>(event.entity)) + " starved.");
    });

    core::EventBus::Get().Subscribe<core::EventCombatResolved>([&log](const core::EventCombatResolved& event) {
        log.entries.emplace_back(
            "Combat: " + std::to_string(static_cast<std::uint32_t>(event.attacker)) +
            " hit " + std::to_string(static_cast<std::uint32_t>(event.defender)) +
            " for " + std::to_string(event.damage));
    });

    core::EventBus::Get().Subscribe<core::EventLLMDecisionReady>([&log](const core::EventLLMDecisionReady& event) {
        log.entries.emplace_back(
            "LLM decision for " + std::to_string(static_cast<std::uint32_t>(event.entity)) +
            ": " + event.decision);
    });
}

void HistorySystem::Update(core::EcsRegistry&, float) {}

} // namespace ant::systems
