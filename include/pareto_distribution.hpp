#pragma once

#include <defs.hpp>

#include <array>
#include <cstdint>
#include <cassert>

namespace alloc_test {

// piecewise-linear approximation to a pareto distribution with finite support
class pareto_distribution {
  public:
    using result_type = std::uint64_t;

  private:
    static constexpr std::array<double, 7> pdf_ = {
        0.262144000000,
        0.393216000000,
        0.245760000000,
        0.081920000000,
        0.015360000000,
        0.001536000000,
        0.000064000000};

    static constexpr std::array<double, 6> cdf_ = {
        0.262144000000,
        0.655360000000,
        0.901120000000,
        0.983040000000,
        0.998400000000,
        0.999936000000};

    std::array<std::uint32_t, 8> offsets_ = {0,0,0,0,0,0,0,0};

  public:
    constexpr pareto_distribution(std::uint32_t count) noexcept {
        offsets_[0] = 0;
        offsets_[7] = count;
        for (unsigned i=0; i<6; ++i) {
            offsets_[i+1] = offsets_[i] + (std::uint32_t)(count * pdf_[6-i]);
            assert(offsets_[i+1] > offsets_[i]);
        }
    }

    template<class Generator>
    FORCE_INLINE result_type operator()(Generator& g) const noexcept {
        const double u = (double)((g()-Generator::min()))/(Generator::max()-Generator::min());
        unsigned idx = 6;
        if (u < cdf_[0])
        	idx = 0;
        else if (u < cdf_[1])
        	idx = 1;
        else if (u < cdf_[2])
        	idx = 2;
        else if (u < cdf_[3])
        	idx = 3;
        else if (u < cdf_[4])
        	idx = 4;
        else if (u < cdf_[5])
        	idx = 5;
        const auto range_size = offsets_[idx+1] - offsets_[idx];
        const std::uint32_t r = (g()-Generator::min()) % range_size;
        return offsets_[idx] + r;
    }
};

} // namespace alloc_test
