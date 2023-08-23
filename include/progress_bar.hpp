#pragma once

#include <fmt/core.h>
#include <fmt/compile.h>

#include <cstdio>

namespace alloc_test {

class progress_bar {
  private:
    unsigned _i = 0;
    unsigned _n = 1;
    unsigned _length = 20;
    unsigned _indent = 0;
    double _p_fraction = 100.0;
    double _l_fraction = 20.0;
    bool _enabled = true;

  public:
    progress_bar(unsigned n, unsigned length = 20, unsigned indent = 0, bool enabled = true)
    : _n{n==0?1:n}
    , _length{length==0?1:length}
    , _indent{indent}
    , _p_fraction{100.0/_n}
    , _l_fraction{(double)_length/_n}
    , _enabled{enabled}
    {
        if (_enabled) {
            fmt::print(FMT_STRING("{0: ^{2}}[{0: ^{1}}] {3: >3.0f}%"), "", _length, _indent, 0.);
            std::fflush(stdout);
        }
    }

    void update() {
        if (!_enabled) return;
        std::printf("\r");
        if (_i+1 < _n) {
            ++_i;
            const auto m = (unsigned)(_i*_l_fraction);
            fmt::print(FMT_STRING("{0: ^{3}}[{0:*^{1}}{0: ^{2}}] {4: >3.0f}%"), "", m, _length-m, _indent, _p_fraction*_i);
        }
        else {
            fmt::print(FMT_STRING("{0: ^{2}}[{0:*^{1}}] {3: >3.0f}%\n"), "", _length, _indent, 100.0);
        }
        std::fflush(stdout);
    }
};

} // namespace alloc_test
