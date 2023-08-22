
#if defined(ALLOC_TEST_STDMALLOC)
#include <test_allocators/std_mallocator.hpp>
#elif defined(ALLOC_TEST_FANCYSTDMALLOC)
#include <test_allocators/fancy_std_mallocator.hpp>
#elif defined(ALLOC_TEST_HWMALLOC)
#include <test_allocators/hw_allocator.hpp>
#elif defined(ALLOC_TEST_MIMALLOC)
#include <test_allocators/mi_allocator.hpp>
#elif defined(ALLOC_TEST_JEMALLOC)
#include <test_allocators/je_allocator.hpp>
#elif defined(ALLOC_TEST_TCMALLOC)
#include <test_allocators/tc_allocator.hpp>
#elif defined(ALLOC_TEST_TSPUMALLOC)
#include <test_allocators/tspu_allocator.hpp>
#endif

#include <mem_access_type.hpp>
#include <run.hpp>
#include <thread_affinity.hpp>

#include <fmt/core.h>

#include <cstdint>

int main() {
    auto alloc = alloc_test::make_test_allocator<std::uint8_t>();

    auto [cores, cpus] = alloc_test::threading::hardware_resources();

    fmt::print(FMT_STRING("hardware resources: cores = {:d},  cpus = {:d}\n"), cores, cpus);

    //static constexpr auto mat = alloc_test::mem_access_type::none;
    //static constexpr auto mat = alloc_test::mem_access_type::single;
    static constexpr auto mat = alloc_test::mem_access_type::full;
    //static constexpr auto mat = alloc_test::mem_access_type::check;

    // run: alloc, iterations, bins, max size exponent, max number of threads
    //                          100000000, 33554432, 16
    alloc_test::run<mat>(alloc, 100000000, 10000000, 16, cores);
    //alloc_test::run<mat>(alloc, 100000000, 10000000, 10, cores);

    return 0;
}
