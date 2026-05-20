#pragma once

struct GLFWwindow;

namespace ant::core {
class EcsRegistry;
}

namespace ant::view {

class DebugGui final {
public:
    void Initialize(GLFWwindow* window);
    void BeginFrame() const;
    void Draw(const core::EcsRegistry& registry) const;
    void EndFrame() const;
    void Shutdown() const;
};

} // namespace ant::view
