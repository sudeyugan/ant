#include "core/memory_pool.h"

#include <algorithm>

namespace ant::core {

MemoryPool::MemoryPool(std::size_t capacity_bytes)
    : buffer_(capacity_bytes > 0 ? std::make_unique<std::byte[]>(capacity_bytes) : nullptr),
      capacity_(capacity_bytes) {}

void MemoryPool::Reset() {
    offset_ = 0;
}

void* MemoryPool::Allocate(std::size_t size_bytes, std::size_t alignment) {
    if (size_bytes == 0 || capacity_ == 0) {
        return nullptr;
    }

    std::size_t current = reinterpret_cast<std::size_t>(buffer_.get()) + offset_;
    std::size_t aligned = AlignForward(current, alignment);
    std::size_t new_offset = aligned - reinterpret_cast<std::size_t>(buffer_.get()) + size_bytes;

    if (new_offset > capacity_) {
        return nullptr;
    }

    offset_ = new_offset;
    return reinterpret_cast<void*>(aligned);
}

std::size_t MemoryPool::Capacity() const {
    return capacity_;
}

std::size_t MemoryPool::Used() const {
    return offset_;
}

std::size_t MemoryPool::AlignForward(std::size_t ptr, std::size_t alignment) const {
    const std::size_t modulo = ptr % alignment;
    if (modulo == 0) {
        return ptr;
    }
    return ptr + (alignment - modulo);
}

} // namespace ant::core
