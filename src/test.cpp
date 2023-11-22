
#if defined(ALLOC_TEST_STDMALLOC)
#include <test_allocators/std_mallocator.hpp>
#elif defined(ALLOC_TEST_PMRMALLOC)
#include <test_allocators/pmr_mallocator.hpp>
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
#elif defined(ALLOC_TEST_HOARDMALLOC)
#include <test_allocators/hoard_allocator.hpp>
#elif defined(ALLOC_TEST_PMIMALLOC)
#include <test_allocators/pmi_allocator.hpp>
#endif

#include <mem_access_type.hpp>
#include <run.hpp>
#include <thread_affinity.hpp>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <cstdint>
#include <string>
#include <unordered_map>

std::unordered_map<std::string, std::size_t> parse_arguments(int argc, char* argv[]) {

    auto [cores, cpus] = alloc_test::threading::hardware_resources();

    auto args = std::unordered_map<std::string, std::size_t>{
        {"iterations", 100000000},
        {"bins",        33554432},
        {"size-exp",          16},
        {"threads",    std::min((std::size_t)cores, alloc_test::max_threads)}};

    auto exit_with_error = [](const std::string& msg, const std::string& id) {
        fmt::print(stderr, FMT_STRING("Error: {:s}: {:s}\n"), msg, id);
        fmt::print(stderr, FMT_STRING("Use '--help' for usage information\n"));
        return std::unordered_map<std::string, std::size_t>{};
    };

    for (int i = 1; i < argc; i += 2) {
        std::string arg = argv[i];

        if (arg == "--help") {
            fmt::print(FMT_STRING("Usage: {:s} [Option]...\n"), argv[0]);
            fmt::print(FMT_STRING("  Options:\n"));
            for (const auto& p : args) {
                fmt::print(FMT_STRING("    --{:<24s}default: {:d}\n"), p.first+" value", p.second);
            }
            fmt::print(FMT_STRING("    --help\n\n"));
            return {};
        }

        if (arg.find("--") != 0) {
            return exit_with_error("invalid argument name", arg);
        }

        if (i + 1 >= argc) {
            return exit_with_error("missing argument value", arg);
        }

        std::string key = arg.substr(2);
        long long value = 0u;
        std::string arg_value = argv[i+1];

        if (args.find(key) == args.end()) {
            return exit_with_error("unrecognized argument", arg);
        }

        try {
            value = std::stoll(arg_value);
        }
        catch (std::out_of_range const&) {
            return exit_with_error("invalid argument value (out of range)", arg_value);
        }
        catch(...) {
            return exit_with_error("invalid argument value", arg_value);
        }
        if (value <= 0u) {
            return exit_with_error("invalid argument value (must be greater than zero)", arg_value);
        }

        args[key] = value;
    }

    if (args["threads"] > alloc_test::max_threads) {
        return exit_with_error("too many threads requested", std::to_string(args["threads"]));
    }

    fmt::print(FMT_STRING("hardware resources: cores = {:d},  cpus = {:d}\n"), cores, cpus);
    return args;
}

int main(int argc, char* argv[]) {
    auto args = parse_arguments(argc, argv);
    if (args.empty()) return 1;

    auto alloc = alloc_test::make_test_allocator<std::uint8_t>();

    fmt::print("arguments: {}\n", args);

    alloc_test::run<>(alloc, args["iterations"], args["bins"], args["size-exp"], args["threads"]);

    return 0;
}
