#pragma once

#include <defs.hpp>
#include <config.hpp>
#include <record.hpp>
#include <task_group.hpp>
#include <clock.hpp>
#include <wrapped_allocator.hpp>
#include <xorshift_rng.hpp>
#include <pareto_distribution.hpp>
#include <zipf_distribution.hpp>
#include <mem_access.hpp>
#include <get_rss.hpp>

#include <fmt/core.h>
#include <fmt/compile.h>

#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>

#define ALLOC_TEST_COLLECT_USER_MAX_ALLOCATED

namespace alloc_test {

inline threading::task_system ts(max_threads, true);

// Struct which holds the allocated memory segment
template<typename Allocator>
struct test_bin {
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    pointer ptr = nullptr;
    std::size_t size = 0u;
    std::size_t reincarnation = 0u;
};

template<mem_access_type M, typename WrappedAllocator>
void run_parallel(WrappedAllocator alloc, record& rec, std::size_t num_iter, std::size_t num_items, std::size_t max_size_exp, std::size_t num_threads) {
    threading::parallel_for::apply(num_threads, &ts,
        [alloc, rec_ptr = &rec, num_iter, num_items, max_size_exp](int thread_id) mutable {
            // get the thread_spefic record
            auto& thread_rec = rec_ptr->thread_records[thread_id];

            // initialize allocator
            alloc.init();

            // read time stamp counter (clock cycle counter)
            thread_rec.rdtsc_begin = __rdtsc();

            // start timer
            auto t0 = now();

            // setup random number generation and statistical distributions
            auto rng32 = xorshift_rng<std::uint32_t>{(std::uint64_t)(33+thread_id)};
            auto rng64 = xorshift_rng<std::uint64_t>{(std::uint64_t)(33+thread_id + 10000)};
            auto idx_dist = pareto_distribution{(std::uint32_t)num_items};
            auto size_dist = zipf_distribution{(std::uint32_t)max_size_exp};

            // create a buffer of test_bin elements - use internal allocator to do that
            using internal_allocator = typename WrappedAllocator::internal_allocator;
            using test_bin_t = test_bin<WrappedAllocator>;
            using base_buffer_allocator = typename std::allocator_traits<internal_allocator>::template rebind_alloc<test_bin_t>;
            auto base_buff = std::vector<test_bin_t, base_buffer_allocator>{num_items, base_buffer_allocator{alloc.get_internal()}};

            // memory access traits and counters
            using access = mem_access<M>;
            std::size_t ctr = 0u;
            std::size_t allocated_size = base_buff.size()*sizeof(test_bin_t);
            std::size_t allocated_size_max = 0u;

            // setup: saturate the buffer (50% probablility of each bin being allocated)
            // use the 32 bit random number to efficiently calculate probability
            for (std::size_t i=0; i<num_items/32; ++i) {
                auto r = rng32();
                for (std::size_t j=0; j<32; ++j) {
                    if ((r >> j) & 1u) {
                        auto& bin = base_buff[32*i + j];
                        const auto size = size_dist(rng64);
                        bin.ptr = alloc.allocate(size);
                        bin.size = size;
                        access::fill(bin);
                        allocated_size += size;
                    }
                }
            }
            alloc.after_setup_phase();
            thread_rec.rdtsc_setup = __rdtsc();
            thread_rec.allocated_after_setup_size = allocated_size;
            thread_rec.rss_max = std::max(thread_rec.rss_max, get_rss());

            // main loop: allocate num_iter bins, measure rss 32 times
            const auto num_iter_by_32 = num_iter >> 5;
            for (int k=0; k<32; ++k) {
                for (std::size_t j=0; j<num_iter_by_32; ++j) {
                    const auto idx = idx_dist(rng32);
                    auto& bin = base_buff[idx];
                    if (bin.ptr) {
                        ctr += access::check(bin);
                        alloc.deallocate(bin.ptr, bin.size);
                        bin.ptr = nullptr;
#ifdef ALLOC_TEST_COLLECT_USER_MAX_ALLOCATED
                        allocated_size -= bin.size;
#endif
                    }
                    else {
                        const auto size = size_dist(rng64);
                        bin.ptr = alloc.allocate(size);
                        bin.size = size;
                        access::fill(bin);
#ifdef ALLOC_TEST_COLLECT_USER_MAX_ALLOCATED
                        allocated_size += size;
                        allocated_size_max = std::max(allocated_size_max, allocated_size);
#endif
                    }
                }
                thread_rec.rss_max = std::max(thread_rec.rss_max, get_rss());
            }
            alloc.after_main_loop_phase();
            thread_rec.rdtsc_main_loop = __rdtsc();
            thread_rec.allocated_max = allocated_size_max;

            // clean up: deallocate all remaining bins
            for (std::size_t idx=0; idx<num_items; ++idx) {
                auto& bin = base_buff[idx];
                auto ptr = bin.ptr;
                if (ptr) {
                    ctr += access::check(bin);
                    alloc.deallocate(ptr, bin.size);
                }
            }
            base_buff = std::vector<test_bin_t, base_buffer_allocator>{0, base_buffer_allocator{alloc.get_internal()}};
            alloc.deinit();
            thread_rec.rdtsc_exit = __rdtsc();
            auto t1 = now();
            alloc.after_cleanup_phase();
            thread_rec.inner_dur = t1-t0;
            thread_rec.rss_max = std::max(thread_rec.rss_max, get_rss());
            fmt::print(FMT_STRING("about to exit thread {:d} ({:d} operations performed) [ctr = {:d}]...\n"), thread_id, num_iter, ctr);
        }
    );
}

// In order to restart on thread 0 we run a number of empty tasks to reset the round-robin
// notification queues
void reset_round_robin(std::size_t num_launched_tasks) {
    threading::task_group g(&ts);
    for (std::size_t j=num_launched_tasks; j<max_threads; ++j) g.run([](){});
    g.wait();
}

template<mem_access_type M, typename Allocator>
void run(Allocator alloc, record& rec, std::size_t num_iter, std::size_t num_items, std::size_t max_size_exp, std::size_t num_threads) {
    // run the test with num_threads
    auto t0 = now();
    run_parallel<M>(alloc, rec, num_iter, num_items, max_size_exp, num_threads);
    auto t1 = now();
    reset_round_robin(num_threads);

    // accumulate statistics
    rec.num_threads = num_threads;
    rec.dur = t1-t0;
    const auto d = to_milliseconds(rec.dur);
    fmt::print(FMT_STRING(
        "{:d} threads made {:d} alloc/dealloc operations in {:.4f} ms ({:.4f} ms per 1 million)\n"),
        num_threads, num_iter*num_threads, d, d*(1000000.0/(num_items*num_threads)));
    // loop over every thread's record
    for (std::size_t i=0; i<num_threads+1; ++i) {
        auto& t_rec = rec.thread_records[i];
        rec.cummulative_dur += (to_milliseconds(t_rec.inner_dur) - rec.cummulative_dur)/(i+1);
        rec.allocated_after_setup_size += t_rec.allocated_after_setup_size;
        rec.allocated_max += t_rec.allocated_max;
        rec.rss_max = std::max(rec.rss_max, t_rec.rss_max);
    }
    rec.rss_after_exiting_all_threads = get_rss();
}

template<mem_access_type M, typename Allocator>
void run(Allocator alloc, std::size_t num_iter, std::size_t num_items, std::size_t max_size_exp, std::size_t num_threads) {
    // record storage
    std::array<record, max_threads> test_records;
    std::array<record, max_threads> fake_records;

    // maximum number of threads is capped by compile time constant
    num_threads = std::min(max_threads, num_threads);
    
    // rebind allocator to be sure to have value_type == std::uint8_t;
    using allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<std::uint8_t>;
    allocator a{alloc};

    // wrap allocator in instrumented allocator
    auto w_alloc = wrap_alloc(a);

    // wrap allocator in instrumented fake allocator: returns one-time allocated buffer resource
    // and a fake_allocator - the buffer resource must be kept alive until the end of the run
    auto [res, f_alloc] = fake_alloc(a, (1ull << max_size_exp));

    // loop over thread configurations
    for (std::size_t n=0; n<num_threads; ++n) {
        run<M>(w_alloc, test_records[n], num_iter, num_items/(n+1), max_size_exp, n+1);
        if constexpr (M!=mem_access_type::check)
            run<M>(f_alloc, fake_records[n], num_iter, num_items/(n+1), max_size_exp, n+1);
    }

    // print statistics summary
    fmt::print(FMT_COMPILE("\nTest summary\n"));
    for (std::size_t n=0; n<num_threads; ++n) {
        auto& test_rec = test_records[n];
        auto& fake_rec = fake_records[n];
        fmt::print(FMT_STRING("{:d}, {:.4f}, {:.4f}, {:.4f}\n"), n, to_milliseconds(test_rec.dur),
            to_milliseconds(fake_rec.dur), to_milliseconds(test_rec.dur - fake_rec.dur));
        fmt::print(FMT_COMPILE("Per-thread stats:\n"));
        for (std::size_t i=0; i<n+1; ++i) {
            auto& t_rec = test_rec.thread_records[i];
            std::uint64_t rdtsc_total = t_rec.rdtsc_exit - t_rec.rdtsc_begin;
            const double rdtsc_norm = 100.0/rdtsc_total;
            fmt::print(FMT_STRING("    {:d}: {:.4f}ms; {:d} ({:.2f} | {:.2f} | {:.2f});\n"),
                i, to_milliseconds(t_rec.inner_dur), rdtsc_total,
                (t_rec.rdtsc_setup - t_rec.rdtsc_begin)*rdtsc_norm,
                (t_rec.rdtsc_main_loop - t_rec.rdtsc_setup)*rdtsc_norm,
                (t_rec.rdtsc_exit - t_rec.rdtsc_main_loop)*rdtsc_norm);
        }
    }
    fmt::print(FMT_COMPILE("\n"));

    // print statistics table
    constexpr const char* mat_str = M==mem_access_type::none ? "none" :
        (M==mem_access_type::single ? "single" : (M==mem_access_type::full ? "full" :
        (M==mem_access_type::check ? "check" : "unknown")));
    fmt::print(FMT_STRING("Short test summary for \'{:s}\' and max_size_exp = {:d}, "
        "num_items = {:d}, num_iter = {:d}, allocated memory access mode = {:s}\n"), allocator_name,
        max_size_exp, num_items, num_iter, mat_str);
    fmt::print(FMT_COMPILE("columns:\n"));
    fmt::print(FMT_COMPILE("thread,duration(ms),duration of void(ms),diff(ms),RSS max(pages),"
        "rssAfterExitingAllThreads(pages),RSS max for void(pages),"
        "rssAfterExitingAllThreads for void(pages),allocatedAfterSetup(app level,bytes),"
        "allocatedMax(app level,bytes),(RSS max<<12)/allocatedMax\n"));
    for (std::size_t n=0; n<num_threads; ++n) {
        auto& test_rec = test_records[n];
        auto& fake_rec = fake_records[n];
        fmt::print(FMT_STRING("{:d},{:d},{:d},{:d},{:d},{:d},{:d},{:d},{:d},{:d},{:f}\n"),
            n+1, (std::size_t)to_milliseconds(test_rec.dur),
            (std::size_t)to_milliseconds(fake_rec.dur),
            (std::int64_t)(to_milliseconds(test_rec.dur)-to_milliseconds(fake_rec.dur)),
            test_rec.rss_max, test_rec.rss_after_exiting_all_threads, fake_rec.rss_max,
            fake_rec.rss_after_exiting_all_threads, test_rec.allocated_after_setup_size,
            test_rec.allocated_max, (test_rec.rss_max << 12) * 1. / test_rec.allocated_max);
    }
}

} // namesapce alloc_test
