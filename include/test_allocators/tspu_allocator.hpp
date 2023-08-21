#pragma once

#include <umpire/ResourceManager.hpp>
#include <umpire/strategy/QuickPool.hpp>
#include <umpire/strategy/ThreadSafeAllocator.hpp>
#include <umpire/TypedAllocator.hpp>

auto& rm = umpire::ResourceManager::getInstance();
using alloc_type = umpire::Allocator;

uint32_t initial_bytes = 1 << 20;
alloc_type u_allocator = rm.getAllocator("HOST");
alloc_type pu_allocator = rm.makeAllocator<umpire::strategy::QuickPool>(
                                    "HOST_pool", u_allocator, initial_bytes);
alloc_type untyped_tspu_allocator = rm.makeAllocator<umpire::strategy::ThreadSafeAllocator>(
                                    "HOST_thread_safe_pool", pu_allocator);

namespace alloc_test {

template<class T>
struct tspu_mallocator {
    using value_type = T;
    using malloc_type = umpire::TypedAllocator<T>;
    malloc_type alloc{untyped_tspu_allocator};
 
    tspu_mallocator() = default;

    template<class U>
    constexpr tspu_mallocator(const tspu_mallocator<U>&) noexcept {}

    [[nodiscard]] T* allocate(std::size_t n) {
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
            throw std::bad_array_new_length();
        if (auto p = static_cast<T*>(alloc.allocate(n * sizeof(T)))) {
            return p;
        }
        throw std::bad_alloc();
    }
 
    void deallocate(T* p, std::size_t n) noexcept {
        alloc.deallocate(p, n);
    }
};

template<class T, class U>
bool operator==(const tspu_mallocator<T>&, const tspu_mallocator<U>&) { return true; }

template<class T, class U>
bool operator!=(const tspu_mallocator<T>&, const tspu_mallocator<U>&) { return false; }


template<typename T>
using test_allocator = tspu_mallocator<T>;

inline constexpr const char* allocator_name = "tspu_malloc";

} // namespace alloc_test

