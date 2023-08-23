#pragma once

#include <jemalloc/jemalloc.h>
#include <new>
#include <limits>
#include "./mallocator.hpp"

namespace alloc_test {

ALLOC_TEST_MALLOCATOR_DEFINITION

template<typename T>
using test_allocator = mallocator<T>;

template<typename T>
auto make_test_allocator() { return test_allocator<T>{}; }

inline constexpr const char* allocator_name = "jemalloc";

} // namespace alloc_test
