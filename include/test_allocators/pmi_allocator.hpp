#pragma once

#include <allocator.hpp>

#include <cstdint>
#include <string>

namespace alloc_test {

template<typename T>
auto make_test_allocator() 
{
    std::size_t mem = 1ull << 31;

    using res_t = resource <context <not_pinned <host_memory <base>> , backend_none> , ex_mimalloc> ;
    auto res = std::make_shared<res_t>(mem);
    // std::cout << "use count of res = " << res.use_count() << std::endl;
    using alloc_t = pmimallocator<T, res_t>;
    alloc_t a{res};
    // std::cout << "use count of res = " << res.use_count() << std::endl;
    return a;

    // resource_builder rb;
    // rb.use_mimalloc();
    // rb.on_host(mem);
    // return pmimallocator<T, decltype(rb.build())>{rb};
}

inline constexpr const char* allocator_name = "pmi_malloc";

} // namespace alloc_test
