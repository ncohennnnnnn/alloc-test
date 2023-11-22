#pragma once

#include <cstdlib>
#include <new>
#include <limits>
#include <memory>
#include <memory_resource>
#include "./mallocator.hpp"

namespace alloc_test {

template<typename T>
auto build() {
    const std::uint32_t buffer_size = 1 << 23;
    auto buffer = std::make_unique<char[]>(buffer_size);
    std::pmr::monotonic_buffer_resource bufferResource(buffer.get(), buffer_size);
    std::pmr::synchronized_pool_resource poolResource(&bufferResource);
    return std::pmr::polymorphic_allocator<T>(&poolResource);
}

template<typename T>
auto make_test_allocator() {
    return build<T>();
}

inline constexpr const char* allocator_name = "pmr_malloc";

} // namespace alloc_test
