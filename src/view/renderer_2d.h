#pragma once

#include <cstdint>

namespace ant::core {
class EcsRegistry;
}

namespace ant::view {

class Renderer2D final {
public:
    void BeginFrame(std::int32_t width, std::int32_t height) const;
    void DrawEntities(const core::EcsRegistry& registry) const;
    void EndFrame() const;
};

} // namespace ant::view
