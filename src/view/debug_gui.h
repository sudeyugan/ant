#pragma once

#include <vector>
#include <string>

struct GLFWwindow;

namespace ant::core {
class EcsRegistry;
}

namespace ant::systems {
struct WorldHistoryLog;
}

namespace ant::view {

class DebugGui final {
public:
    void Initialize(GLFWwindow* window);
    void BeginFrame() const;
    void Draw(core::EcsRegistry& registry, const systems::WorldHistoryLog& history) const;
    void EndFrame() const;
    void Shutdown() const;

private:
    mutable std::vector<std::string> chronicles_{};
};

} // namespace ant::view
