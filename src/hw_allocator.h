
/* -------------------------------------------------------------------------------
 * Added file     Jul-04-2023
 * -------------------------------------------------------------------------------
 *
 * Memory allocator tester -- hwmalloc allocator
 *
 * -------------------------------------------------------------------------------*/

#pragma once

#include <iostream>
#include <utility>

#include "test_common.h"

#include <hwmalloc/allocator.hpp>
#include <hwmalloc/heap.hpp>

struct context
{
    int m = 42;
    context() { std::cout << "context constructor" << std::endl; }
    ~context() { std::cout << "context destructor" << std::endl; }

    struct region
    {
        struct handle_type
        {
            void* ptr;
        };

        void* ptr = nullptr;

        region(void* p) noexcept
        : ptr{p}
        {
        }

        region(region const&) = delete;

        region(region&& other) noexcept
        : ptr{std::exchange(other.ptr, nullptr)}
        {
        }

        ~region(){}

        handle_type get_handle(std::size_t offset, std::size_t /*size*/) const noexcept
        {
            return {(void*)((char*)ptr + offset)};
        }
    };
};

auto
register_memory(context&, void* ptr, std::size_t)
{
    return context::region{ptr};
}

struct dummy_init {
    dummy_init() {
        std::cout << "Dummy init being called" << std::endl;
        auto ctx = new context();
        auto new_heap = hwmalloc::heap<context>::get_instance(ctx);
    }
};

static dummy_init dummy;

template<typename T>
class hwmallocAllocatorForTest : public hwmalloc::allocator<T,hwmalloc::heap<context>>
{
    using base = hwmalloc::allocator<T,hwmalloc::heap<context>>;
    using pointer = hwmalloc::hw_ptr<T, hwmalloc::heap<context>::block_type>;
    ThreadTestRes* testRes;

public:
    template<typename U>
    struct other_alloc
    {
        using other = hwmallocAllocatorForTest<U>;
    };
    template<typename U>
    using rebind = other_alloc<U>;

public:
    hwmallocAllocatorForTest( ThreadTestRes* testRes_ ) {
        testRes = testRes_;
    }

    using is_fake = std::false_type;
    using is_fancy = std::true_type;
    static constexpr bool isFake() { return false; }
    static constexpr bool isFancy() { return true; }

    static constexpr const char* name() { return "hwmalloc allocator"; }

    void init(){}
    void deallocate(pointer ptr) {base::deallocate(ptr, sizeof(ptr)); } // got rid of the const&
    void deinit(){}

    // next calls are to get additional stats of the allocator, etc, if desired
    void doWhateverAfterSetupPhase() {}
    void doWhateverAfterMainLoopPhase() {}
    void doWhateverAfterCleanupPhase() {}

    ThreadTestRes* getTestRes() { return testRes; }

};

