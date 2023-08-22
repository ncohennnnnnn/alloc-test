#pragma once

#include <hwmalloc/allocator.hpp>
#include <hwmalloc/heap.hpp>

namespace alloc_test {

struct context{
    struct region {
        struct handle_type {
            void* ptr;
        };

        void* ptr = nullptr;

        region(void* p) noexcept : ptr{p} {}

        region(region const&) = delete;

        region(region&& other) noexcept : ptr{std::exchange(other.ptr, nullptr)} {}

        ~region(){}

        handle_type get_handle(std::size_t offset, std::size_t /*size*/) const noexcept {
            return {(void*)((char*)ptr + offset)};
        }
    };
};

auto
register_memory(context&, void* ptr, std::size_t) {
    return context::region{ptr};
}

template<typename T>
using test_allocator = hwmalloc::allocator<T, hwmalloc::heap<context>>;

template<typename T>
auto make_test_allocator() { return test_allocator<T>{}; }

inline constexpr const char* allocator_name = "hwmalloc";

} // namespace alloc_test
