#pragma once

#include <cstdint>

namespace ant::components {

struct Health {
    float current{100.0f};
    float max{100.0f};
};

struct Hunger {
    float value{100.0f};
    float decay_rate{1.0f};
};

struct MemoryRef {
    std::uint32_t vector_db_id{0};
};

} // namespace ant::components
