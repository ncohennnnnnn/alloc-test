#pragma once


#include <chrono>

namespace alloc_test {

using clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::time_point<clock>;
using duration = clock::duration;
using milliseconds = std::chrono::duration<double, std::chrono::milliseconds::period>;
using microseconds = std::chrono::duration<double, std::chrono::microseconds::period>;

inline double to_milliseconds(const duration& d) noexcept {
    return milliseconds(d).count();
}

inline double to_microseconds(const duration& d) noexcept {
    return microseconds(d).count();
}

inline time_point now() noexcept { return clock::now(); }

} // alloc_test
