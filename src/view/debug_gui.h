#pragma once

namespace ant::core {
class EcsRegistry;
}

namespace ant::view {

class DebugGui final {
public:
    void Draw(const core::EcsRegistry& registry) const;
};

} // namespace ant::view
