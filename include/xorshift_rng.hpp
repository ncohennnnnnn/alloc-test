#pragma once

#include <defs.hpp>

#include <cstdint>
#include <limits>

namespace alloc_test {

// simple random number generator using xor operations
template<class UIntType, std::uint64_t a, std::uint64_t b1, std::uint64_t b2, std::uint64_t b3>
class xorshift_rng_engine;

template<std::uint64_t a, std::uint64_t b1, std::uint64_t b2, std::uint64_t b3>
class xorshift_rng_engine<std::uint32_t, a, b1, b2, b3> {
  public:
    using result_type = std::uint32_t;

  private:
    std::uint64_t seed_ = 0;

  public:
    xorshift_rng_engine() = default;

    xorshift_rng_engine(std::uint64_t s) noexcept : seed_{s} {}

    void seed(std::uint64_t s  = 0) noexcept { seed_ = s; }

    static constexpr result_type min() noexcept { return 0; }

    static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    FORCE_INLINE result_type operator()() noexcept {
        std::uint64_t ret = seed_ * a;
        seed_ ^= seed_ >> b1;
        seed_ ^= seed_ << b2;
        seed_ ^= seed_ >> b3;
        return ret >> 32ull;
    }
};

template<std::uint64_t a, std::uint64_t b1, std::uint64_t b2, std::uint64_t b3>
class xorshift_rng_engine<std::uint64_t, a, b1, b2, b3> {
  public:
    using result_type = std::uint64_t;

  private:
    xorshift_rng_engine<std::uint32_t, a, b1, b2, b3> rng_32_;

  public:
    xorshift_rng_engine() = default;

    xorshift_rng_engine(std::uint64_t s) noexcept : rng_32_{s} {}

    void seed(std::uint64_t s  = 0) noexcept { rng_32_.seed(s); }

    static constexpr result_type min() noexcept { return 0; }

    static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    FORCE_INLINE result_type operator()() noexcept {
        uint64_t ret = rng_32_();
		ret <<= 32;
		return ret + rng_32_();
    }
};

template<typename UIntType>
using xorshift_rng = xorshift_rng_engine<UIntType, 0xd989bcacc137dcd5ull, 11, 31, 18>;

} // namespace alloc_test
