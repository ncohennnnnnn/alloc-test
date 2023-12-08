#pragma once

#include <cstdint>
#include <string>
//
#include <pmimalloc/allocator.hpp>

namespace alloc_test {

    template <typename T>
    auto make_test_allocator()
    {
    /*
Minimum size for good functioning : 1 << 31
Maximum size for pinning : 1 << 30
*/
    std::size_t mem = 1ull << 32;
    using res_t   = simple<resource <context <not_pinned <host_memory <base>> , backend_none> , ext_mimalloc>> ;
    using alloc_t = pmimallocator<T, res_t>;
    auto res = std::make_shared<res_t>(mem);
    alloc_t a{res};
    return a;
}

    inline constexpr const char* allocator_name = "pmi_malloc";

}    // namespace alloc_test
