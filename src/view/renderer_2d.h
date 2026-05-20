#pragma once

#include <cstdint>

namespace ant::core {
class EcsRegistry;
}

namespace ant::spatial {
class ResourceGrid;
}

namespace ant::view {

struct RenderSettings {
    bool draw_sugar{true};
    bool draw_entities{true};
    bool draw_vision{true};
    bool draw_targets{true};
    std::uint32_t max_debug_entities{8};
};

class Renderer2D final {
public:
    void BeginFrame(std::int32_t framebuffer_width, std::int32_t framebuffer_height,
                    float world_width, float world_height) const;
    void DrawWorld(const spatial::ResourceGrid& grid,
                   const core::EcsRegistry& registry,
                   const RenderSettings& settings) const;
    void EndFrame() const;
};

} // namespace ant::view
