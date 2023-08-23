#pragma once

#include <fancy_allocator_adaptor.hpp>

#include <cstdlib>
#include <new>
#include <limits>
#include "./mallocator.hpp"

namespace alloc_test {

using std::malloc;
using std::free;

ALLOC_TEST_MALLOCATOR_DEFINITION

template<typename T>
using test_allocator = fancy_allocator_adaptor<mallocator<T>>;

template<typename T>
auto make_test_allocator() { return test_allocator<T>{}; }

inline constexpr const char* allocator_name = "fancy_std_malloc";

} // namespace alloc_test

