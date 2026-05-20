#pragma once

#include <cstdint>
#include <string>

namespace ant::core {

struct SimulationConfig {
    std::uint32_t map_width{128};
    std::uint32_t map_height{72};
    float max_sugar{10.0f};
    float growback_rate{0.05f};
    std::uint32_t growback_interval{1};
    int vision{6};
    float metabolism_rate{0.2f};
    bool auto_step{true};
    std::uint32_t summary_interval{365};
    float llm_decision_cooldown{5.0f};
    std::string deepseek_base_url{"https://api.deepseek.com"};
    std::string deepseek_api_key{};
    std::string deepseek_model{"deepseek-v4-pro"};
};

SimulationConfig LoadSimulationConfig(const std::string& path);

} // namespace ant::core
