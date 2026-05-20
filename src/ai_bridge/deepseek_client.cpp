#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "ai_bridge/deepseek_client.h"

#include <algorithm>
#include <cctype>
#include <memory>

#include <httplib.h>
#include <nlohmann/json.hpp>

namespace ant::ai_bridge {

struct ParsedUrl {
    bool https{true};
    std::string host{};
    int port{443};
    std::string base_path{};
};

static bool ParseBaseUrl(const std::string& base_url, ParsedUrl& out, std::string& error) {
    std::string url = base_url;
    if (url.rfind("https://", 0) == 0) {
        out.https = true;
        url = url.substr(8);
        out.port = 443;
    } else if (url.rfind("http://", 0) == 0) {
        out.https = false;
        url = url.substr(7);
        out.port = 80;
    } else {
        error = "DeepSeek base_url must start with http:// or https://";
        return false;
    }

    const auto slash_pos = url.find('/');
    std::string host_port = url;
    if (slash_pos != std::string::npos) {
        host_port = url.substr(0, slash_pos);
        out.base_path = url.substr(slash_pos);
    }

    if (host_port.empty()) {
        error = "DeepSeek base_url missing host";
        return false;
    }

    const auto colon_pos = host_port.find(':');
    if (colon_pos != std::string::npos) {
        out.host = host_port.substr(0, colon_pos);
        const std::string port_str = host_port.substr(colon_pos + 1);
        if (port_str.empty() || !std::all_of(port_str.begin(), port_str.end(), ::isdigit)) {
            error = "DeepSeek base_url has invalid port";
            return false;
        }
        out.port = std::stoi(port_str);
    } else {
        out.host = host_port;
    }

    if (out.host.empty()) {
        error = "DeepSeek base_url missing host";
        return false;
    }

    return true;
}

std::optional<std::string> RequestChatCompletion(
    const DeepSeekConfig& config,
    const std::string& system_prompt,
    const std::string& user_prompt,
    std::string& error_out) {
    if (config.api_key.empty()) {
        error_out = "DeepSeek API key is empty";
        return std::nullopt;
    }

    ParsedUrl parsed{};
    if (!ParseBaseUrl(config.base_url, parsed, error_out)) {
        return std::nullopt;
    }

    nlohmann::json body;
    body["model"] = config.model;
    body["messages"] = nlohmann::json::array({
        {{"role", "system"}, {"content", system_prompt}},
        {{"role", "user"}, {"content", user_prompt}}
    });
    body["temperature"] = config.temperature;
    body["max_tokens"] = config.max_tokens;

    const std::string body_text = body.dump();
    const std::string endpoint = parsed.base_path.empty()
        ? "/chat/completions"
        : parsed.base_path + "/chat/completions";

    const std::string scheme = parsed.https ? "https" : "http";
    const std::string scheme_host_port =
        scheme + "://" + parsed.host + ":" + std::to_string(parsed.port);
    httplib::Client client(scheme_host_port);
    client.set_connection_timeout(10, 0);
    client.set_read_timeout(30, 0);
    client.set_write_timeout(30, 0);

    httplib::Headers headers = {
        {"Authorization", "Bearer " + config.api_key},
        {"Content-Type", "application/json"}
    };

    auto res = client.Post(endpoint.c_str(), headers, body_text, "application/json");
    if (!res) {
        error_out = "DeepSeek HTTP request failed";
        return std::nullopt;
    }

    if (res->status < 200 || res->status >= 300) {
        error_out = "DeepSeek HTTP status " + std::to_string(res->status) + ": " + res->body;
        return std::nullopt;
    }

    nlohmann::json response;
    try {
        response = nlohmann::json::parse(res->body);
    } catch (const std::exception& ex) {
        error_out = std::string("DeepSeek response parse error: ") + ex.what();
        return std::nullopt;
    }

    if (!response.contains("choices") || response["choices"].empty()) {
        error_out = "DeepSeek response missing choices";
        return std::nullopt;
    }

    const auto& message = response["choices"][0]["message"];
    if (!message.contains("content")) {
        error_out = "DeepSeek response missing content";
        return std::nullopt;
    }

    return message["content"].get<std::string>();
}

} // namespace ant::ai_bridge
