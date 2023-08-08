#pragma once

#include <defs.hpp>
#include <mem_access_type.hpp>
#include <xorshift_rng.hpp>
#include <to_address.hpp>

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <new>
#include <memory>

namespace alloc_test {

template<mem_access_type M = mem_access_type::none>
struct mem_access {
    template<typename Bin>
    static void fill(Bin& bin) {}

    template<typename Bin>
    static std::size_t check(Bin& bin) { return 0u; }
};

template<>
struct mem_access<mem_access_type::single> {
    template<typename Bin>
    static void fill(Bin& bin) {
        bin.ptr[bin.size/2] = (std::uint8_t)bin.size;
    }

    template<typename Bin>
    static std::size_t check(Bin& bin) {
        return bin.ptr[bin.size/2];
    }
};

template<>
struct mem_access<mem_access_type::full> {
    template<typename Bin>
    static void fill(Bin& bin) {
        const auto fill_byte = (std::uint8_t)bin.size;
        std::memset(to_address(bin.ptr), (int)fill_byte, bin.size);
    }

    template<typename Bin>
    static std::size_t check(Bin& bin) {
        std::size_t ctr = 0u;
        for (std::size_t i=0; i<bin.size; ++i)
            ctr += bin.ptr[i];
        return ctr;
    }
};

template<>
struct mem_access<mem_access_type::check> {
    template<typename Bin>
    static void fill(Bin& bin) {
        for_each_byte(bin.ptr, bin.size, bin.reincarnation,
            [](std::uint8_t& data, std::uint8_t ref) {
                data = ref;
            }
        );
    }

    template<typename Bin>
    static std::size_t check(Bin& bin) {
        for_each_byte(bin.ptr, bin.size, bin.reincarnation,
            [](std::uint8_t& data, std::uint8_t ref) {
                if (data != ref)
                    throw std::bad_alloc();
            }
        );
        return 0u;
    }

    // Loop over each byte as uint8_t and invoke the callback f with a reference to the uint8_t and and
    // a uuid value generated from a pseudo-random number generator
    // Signature of F: void(std::uint8_t&, 
    template<typename Pointer, typename F>
    static void for_each_byte(Pointer ptr, std::size_t size, std::size_t reincarnation, F&& f) {
        static_assert(std::is_same_v<typename std::pointer_traits<Pointer>::element_type, std::uint8_t>);
        // address of pointer
        std::uint8_t* p = to_address(ptr);
        // setup unique seed
        const std::uint64_t seed = ((std::uintptr_t)p) ^ ((std::uintptr_t)size << 32) ^ reincarnation;
        auto rng32 = xorshift_rng<std::uint32_t>{seed};
        // first part (multiple of 4 bytes/32 bits)
        for (std::size_t i=0; i<(size>>2); ++i) {
            const auto r = rng32();
            f(p[i*4+0], (std::uint8_t)r);
            f(p[i*4+1], (std::uint8_t)(r>>8));
            f(p[i*4+2], (std::uint8_t)(r>>16));
            f(p[i*4+3], (std::uint8_t)(r>>24));
        }
        // remainder (only if last 2 bits are non-zero i.e. size is not divisible by 4)
        if (size & 3ul) {
            // fill rest: p += (size/4)*4
            p += (size >> 2) << 2;
            auto last = rng32();
            for (std::size_t i=0; i<(size & 3ul); ++i) {
                f(p[i], (std::uint8_t)last);
                last >>= 8;
            }
        }
    }
};

} // namespace alloc_test
