#pragma once

#include <type_traits>

namespace alloc_test {

template<class T>
constexpr T* to_address(T* p) noexcept {
    static_assert(!std::is_function_v<T>);
    return p;
}
 
template<class T>
constexpr auto to_address(const T& p) noexcept {
    //if constexpr (requires{ std::pointer_traits<T>::to_address(p); })
    //    return std::pointer_traits<T>::to_address(p);
    //else
        return to_address(p.operator->());
}

} // namespace alloc_test
