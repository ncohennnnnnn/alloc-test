#pragma once

#include <mimalloc.h>

namespace alloc_test {

template<typename T>
using test_allocator = mi_stl_allocator<T>;

inline constexpr const char* allocator_name = "mimalloc";

} // namespace alloc_test
