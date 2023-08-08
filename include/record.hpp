#include <defs.hpp>
#include <config.hpp>
#include <clock.hpp>

#include <cstdint>

namespace alloc_test {

struct thread_record {
    // thread identifier
    unsigned thread_id = 0;

    // wall-clock time
    duration inner_dur = duration{0};

    // max rss level
    std::size_t rss_max = 0u;

    // CPU cycles
    std::uint64_t rdtsc_begin = 0u;
    std::uint64_t rdtsc_setup = 0u;
    std::uint64_t rdtsc_main_loop = 0u;
    std::uint64_t rdtsc_exit = 0u;

    // allocated memory
    std::size_t allocated_after_setup_size = 0u;
    std::size_t allocated_max = 0u;
};

struct record {
    // number of threads
    std::size_t num_threads = 1;

    // wall-clock time
    duration dur = duration{0};
    // mean wall-clock time per thread in ms
    double cummulative_dur = 0;

    // max rss level
    std::size_t rss_max = 0;
    std::size_t rss_after_exiting_all_threads = 0;

    // allocated memory
    std::size_t allocated_after_setup_size = 0u;
    std::size_t allocated_max = 0u;

    // per-thread records
    thread_record thread_records[max_threads];
};

} // namespace alloc_test
