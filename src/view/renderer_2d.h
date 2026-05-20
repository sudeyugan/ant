#pragma once

namespace ant::core {
class EcsRegistry;
}

namespace ant::view {

class Renderer2D final {
public:
    void BeginFrame() const;
    void DrawEntities(const core::EcsRegistry& registry) const;
    void EndFrame() const;
};

} // namespace ant::view
