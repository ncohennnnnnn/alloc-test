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
Minimum size for mem : max( 30 , 29 + ceil(log_2(nb_threads)) )
 ____________________________________________
|threads | min size      threads | min size  |
|------------------      ------------------  |
|   1    |   30            12    |   34      |    
|   2    |   31            13    |   34      |       
|   3    |   32            14    |   34      |
|   4    |   32            15    |   34      |
|   5    |   33            16    |   34      |
|   6    |   33            17    |   35      |
|   7    |   33                  .           |
|   8    |   33                  .           |
|   9    |   34                  .           |
|  10    |   34            32    |   35      |    
|  11    |   34                              |
----------------------------------------------

Maximum size for mem : 35 (max of mmap on my machine)
*/
        std::size_t mem = 1ull << 32;
        using res_t = resource<context<not_pinned<host_memory<base>>, backend_none>, ext_mimalloc>;
        using alloc_t = pmimallocator<T, res_t>;
        auto res = std::make_shared<res_t>(mem);
        alloc_t a{res};
        return a;
    }

    inline constexpr const char* allocator_name = "pmi_malloc";

}    // namespace alloc_test
