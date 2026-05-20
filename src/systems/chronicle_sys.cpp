#include "chronicle_sys.h"

#include <algorithm>
#include <sstream>

#include "core/ecs_registry.h"
#include "core/event_bus.h"
#include "core/events.h"
#include "core/simulation_config.h"
#include "systems/history_sys.h"
#include "ai_bridge/llm_summary_client.h"
#include "components/agent.h"
#include "components/chronicle.h"

namespace ant::systems {

ChronicleSystem::ChronicleSystem() {
    ant::core::EventBus::Get().Subscribe<ant::core::EventEntityStarved>([this](const ant::core::EventEntityStarved&) {
        ++starved_this_period_;
    });
    ant::core::EventBus::Get().Subscribe<ant::core::EventOrganizationFormed>([this](const ant::core::EventOrganizationFormed&) {
        ++org_formed_this_period_;
    });
}

void ChronicleSystem::Update(
    ant::core::EcsRegistry& registry,
    const ant::core::SimulationConfig& config,
    const WorldHistoryLog& history,
    ant::ai_bridge::LLMSummaryClient& summary_client,
    uint64_t tick
) {
    if (config.summary_interval == 0) return;
    
    if (tick > 0 && tick % config.summary_interval == 0) {
        // Trigger summary generation
        auto& reg = registry.Raw();
        
        // Gather statistics
        int alive_count = 0;
        float total_wealth = 0.0f;
        
        auto view = reg.view<ant::components::Wealth>();
        for (auto e : view) {
            if (reg.valid(e)) {
                alive_count++;
                total_wealth += reg.get<ant::components::Wealth>(e).coins;
            }
        }
        
        float avg_wealth = alive_count > 0 ? total_wealth / alive_count : 0.0f;
        
        // Build key events text (last 10)
        std::ostringstream key_events;
        const int max_events = 10;
        const int total = static_cast<int>(history.entries.size());
        const int start = std::max(0, total - max_events);
        for (int i = start; i < total; ++i) {
            key_events << "- " << history.entries[i] << "\n";
        }

        summary_client.RequestSummary(
            alive_count,
            starved_this_period_,
            org_formed_this_period_,
            avg_wealth,
            key_events.str(),
            tick
        );
        
        // Reset counters
        starved_this_period_ = 0;
        org_formed_this_period_ = 0;
    }
}

}  // namespace ant::systems
