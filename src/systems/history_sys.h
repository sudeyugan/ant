#pragma once

#include <string>
#include <vector>

namespace ant::core {
class EcsRegistry;
}

namespace ant::systems {

struct WorldHistoryLog {
    std::vector<std::string> entries{};
};

class HistorySystem final {
public:
    static void Register(WorldHistoryLog& log);
    static void Update(core::EcsRegistry& registry, float dt);
};

} // namespace ant::systems
