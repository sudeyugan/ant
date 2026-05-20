#pragma once

#include <optional>
#include <string>

namespace ant::ai_bridge {

struct DeepSeekConfig {
    std::string base_url{"https://api.deepseek.com"};
    std::string api_key{};
    std::string model{"deepseek-v4-pro"};
    float temperature{0.7f};
    int max_tokens{512};
};

std::optional<std::string> RequestChatCompletion(
    const DeepSeekConfig& config,
    const std::string& system_prompt,
    const std::string& user_prompt,
    std::string& error_out);

} // namespace ant::ai_bridge
