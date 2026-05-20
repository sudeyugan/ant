#pragma once

#include <cstdint>

namespace ant::core {
    class EcsRegistry;
    struct SimulationConfig;
}

namespace ant::ai_bridge {
    class LLMSummaryClient;
}

namespace ant::systems {

struct WorldHistoryLog;

class ChronicleSystem {
public:
    ChronicleSystem();

    void Update(ant::core::EcsRegistry& registry,
                const ant::core::SimulationConfig& config,
                const WorldHistoryLog& history,
                ant::ai_bridge::LLMSummaryClient& summary_client,
                uint64_t tick);

private:
    uint64_t last_summary_tick_{};
    int org_formed_this_period_{};
    int starved_this_period_{};
};

}  // namespace ant::systems
