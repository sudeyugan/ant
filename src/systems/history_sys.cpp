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

    // Emergence events
    core::EventBus::Get().Subscribe<core::EventFamilyFormed>([&log](const core::EventFamilyFormed& event) {
        log.entries.emplace_back(
            "Family #" + std::to_string(event.family_id) + " formed by " + 
            std::to_string(static_cast<std::uint32_t>(event.parent1)) + " and " +
            std::to_string(static_cast<std::uint32_t>(event.parent2)));
    });

    core::EventBus::Get().Subscribe<core::EventOrganizationFormed>([&log](const core::EventOrganizationFormed& event) {
        log.entries.emplace_back(
            "Organization #" + std::to_string(event.org_id) + " formed with " +
            std::to_string(event.member_count) + " members, led by " +
            std::to_string(static_cast<std::uint32_t>(event.leader)));
    });

    core::EventBus::Get().Subscribe<core::EventOrganizationDissolved>([&log](const core::EventOrganizationDissolved& event) {
        log.entries.emplace_back(
            "Organization #" + std::to_string(event.org_id) + " dissolved");
    });

    core::EventBus::Get().Subscribe<core::EventLeadershipTransfer>([&log](const core::EventLeadershipTransfer& event) {
        log.entries.emplace_back(
            "Organization #" + std::to_string(event.org_id) + " leadership transferred from " +
            std::to_string(static_cast<std::uint32_t>(event.old_leader)) + " to " +
            std::to_string(static_cast<std::uint32_t>(event.new_leader)));
    });

    // Brain events
    core::EventBus::Get().Subscribe<core::EventBrainGranted>([&log](const core::EventBrainGranted& event) {
        log.entries.emplace_back(
            "Entity " + std::to_string(static_cast<std::uint32_t>(event.entity)) + 
            " granted LLM intelligence (ID: " + std::to_string(event.llm_id) + ")");
    });

    core::EventBus::Get().Subscribe<core::EventBrainRevoked>([&log](const core::EventBrainRevoked& event) {
        log.entries.emplace_back(
            "Entity " + std::to_string(static_cast<std::uint32_t>(event.entity)) + " intelligence revoked");
    });
}

void HistorySystem::Update(core::EcsRegistry&, float) {}

} // namespace ant::systems
