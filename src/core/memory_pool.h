#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <utility>

namespace ant::core {

class MemoryPool final {
public:
    explicit MemoryPool(std::size_t capacity_bytes);
    ~MemoryPool() = default;

    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
    MemoryPool(MemoryPool&&) = delete;
    MemoryPool& operator=(MemoryPool&&) = delete;

    void Reset();
    void* Allocate(std::size_t size_bytes, std::size_t alignment = alignof(std::max_align_t));

    template <typename T, typename... Args>
    T* Create(Args&&... args) {
        void* memory = Allocate(sizeof(T), alignof(T));
        if (!memory) {
            return nullptr;
        }
        return new (memory) T(std::forward<Args>(args)...);
    }

    std::size_t Capacity() const;
    std::size_t Used() const;

private:
    std::size_t AlignForward(std::size_t ptr, std::size_t alignment) const;

    std::unique_ptr<std::byte[]> buffer_;
    std::size_t capacity_{0};
    std::size_t offset_{0};
};

} // namespace ant::core
