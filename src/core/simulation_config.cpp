#include "core/simulation_config.h"

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

namespace ant::core {

SimulationConfig LoadSimulationConfig(const std::string& path) {
    SimulationConfig config{};
    std::ifstream file(path);
    if (!file) {
        std::cerr << "SimulationConfig: cannot open file: " << path << "\n";
        return config;
    }

    try {
        nlohmann::json json;
        file >> json;
        config.map_width = json.value("map_width", config.map_width);
        config.map_height = json.value("map_height", config.map_height);
        config.max_sugar = json.value("max_sugar", config.max_sugar);
        config.growback_rate = json.value("growback_rate", config.growback_rate);
        config.growback_interval = json.value("growback_interval", config.growback_interval);
        config.vision = json.value("vision", config.vision);
        config.metabolism_rate = json.value("metabolism_rate", config.metabolism_rate);
        config.auto_step = json.value("auto_step", config.auto_step);
        config.summary_interval = json.value("summary_interval", config.summary_interval);
        config.llm_decision_cooldown = json.value("llm_decision_cooldown", config.llm_decision_cooldown);
        config.deepseek_base_url = json.value("deepseek_base_url", config.deepseek_base_url);
        config.deepseek_api_key = json.value("deepseek_api_key", config.deepseek_api_key);
        config.deepseek_model = json.value("deepseek_model", config.deepseek_model);
    } catch (const std::exception& ex) {
        std::cerr << "SimulationConfig: failed to parse " << path << ": " << ex.what() << "\n";
    }

    return config;
}

} // namespace ant::core
