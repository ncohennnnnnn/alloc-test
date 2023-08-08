#pragma once

#include <defs.hpp>

#include <cstdint>
#include <cassert>

#ifdef _MSC_VER // For Microsoft Visual C++
#include <intrin.h>
#endif

namespace alloc_test {
    
// piecewise-linear approximation to a Zipf distribution with finite support
// (sample in [1, 2^max_exponent] using bit level alchemy
class zipf_distribution {
  public:
    using result_type = std::uint64_t;

  private:
    std::uint32_t idx_bits_ = 0;

  public:
    constexpr zipf_distribution(std::uint32_t max_exponent) noexcept : idx_bits_{max_exponent-3} {
        assert(max_exponent >= 3);
    }

    template<class Generator>
    FORCE_INLINE result_type operator()(Generator& g) const noexcept {
        std::uint64_t r = g()-Generator::min();
        const std::uint64_t idx = ctz((r & ((((std::uint64_t)1) << idx_bits_) - 1)) + 1) + 2;
        const std::uint64_t mask = (((std::uint64_t)1) << idx) - 1;
        return ((r >> idx_bits_) & mask) + 1 + (((std::uint64_t)1) << idx);
    }

  private:
    FORCE_INLINE static constexpr std::uint64_t ctz(std::uint32_t num) noexcept {
#if defined(__GNUC__) || defined(__clang__) // For GCC or Clang
        return __builtin_ctz(num);
#elif defined(_MSC_VER) // For Microsoft Visual C++
        unsigned long index;
        _BitScanForward(&index, num);
        return index;
#else
        // If the compiler doesn't support the built-in functions, use a generic method
        unsigned int count = 0;
        while ((num & 1) == 0) {
            num >>= 1;
            ++count;
        }
        return count;
#endif
    }
};

} // namespace alloc_test
