#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ant::components {

struct ChronicleReport {
    uint64_t period_end_tick{};
    std::string title{};
    int alive_count{};
    int starved_count{};
    int orgs_formed{};
    float avg_wealth{};
    std::vector<std::string> key_events{};
    std::string llm_report{};
};

}  // namespace ant::components
