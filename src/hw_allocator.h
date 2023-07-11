
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

class hwmallocAllocatorForTest : public hwmalloc::allocator<uint8_t,hwmalloc::heap<context>>
{
    using base = hwmalloc::allocator<uint8_t,hwmalloc::heap<context>>;
	ThreadTestRes* testRes;

public:
	hwmallocAllocatorForTest( ThreadTestRes* testRes_ ) { testRes = testRes_; }
	static constexpr bool isFake() { return false; }

	static constexpr const char* name() { return "hwmalloc allocator"; }

	void init(){}
    void deallocate(pointer const& ptr) {base::deallocate(ptr, sizeof(ptr)); }
	void deinit(){}

    // next calls are to get additional stats of the allocator, etc, if desired
	void doWhateverAfterSetupPhase() {}
	void doWhateverAfterMainLoopPhase() {}
	void doWhateverAfterCleanupPhase() {}

	ThreadTestRes* getTestRes() { return testRes; }

};
