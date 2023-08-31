#pragma once

#include <umpire/ResourceManager.hpp>
#include <umpire/strategy/QuickPool.hpp>
#include <umpire/strategy/ThreadSafeAllocator.hpp>
#include <umpire/TypedAllocator.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>

namespace alloc_test {

template<class T>
class u_allocator : public umpire::TypedAllocator<T> {
  public:
    using base = umpire::TypedAllocator<T>;
    using alloc_traits = std::allocator_traits<base>;
    using value_type = typename alloc_traits::value_type;
    using pointer = typename alloc_traits::pointer;
    using const_pointer = typename alloc_traits::const_pointer;
    using void_pointer = typename alloc_traits::void_pointer;
    using const_void_pointer = typename alloc_traits::const_void_pointer;
    using difference_type = typename alloc_traits::difference_type;
    using size_type = typename alloc_traits::size_type;

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::false_type;

    template<typename U>
    struct rebind {
        using other = u_allocator<U>;
    };

    u_allocator(base b) : base{b} {}
    u_allocator(const u_allocator&) = default;
    u_allocator(u_allocator&&) = default;
    u_allocator& operator=(const u_allocator&) = default;
    u_allocator& operator=(u_allocator&&) = default;

    inline friend bool operator==(const u_allocator& lhs, const u_allocator& rhs) noexcept {
        return static_cast<const base&>(lhs) == static_cast<const base&>(rhs);
    }

    inline friend bool operator!=(const u_allocator& lhs, const u_allocator& rhs) noexcept {
        return !(lhs==rhs);
    }

    u_allocator select_on_container_copy_construction() {
        return *this;
    }
};

template<typename Alloc>
auto adopt_quick_pool(umpire::ResourceManager& rm, Alloc a) {
    const std::string name = a.getName() + "_pool";
    static constexpr std::uint32_t initial_bytes = 1 << 23;
    return rm.makeAllocator<umpire::strategy::QuickPool>(name.c_str(), a, initial_bytes);
}

template<typename Alloc>
auto adopt_thread_safety(umpire::ResourceManager& rm, Alloc a) {
    const std::string name = a.getName() + "_thread_safe";
    return rm.makeAllocator<umpire::strategy::ThreadSafeAllocator>(name.c_str(), a);
}

template<typename T, typename A = umpire::Allocator>
struct allocator_builder {

    umpire::ResourceManager& rm;
    A alloc;

    allocator_builder()
    : rm{umpire::ResourceManager::getInstance()}
    , alloc{rm.getAllocator("HOST")}
    {}

    allocator_builder(const allocator_builder&) = delete;
    allocator_builder& operator=(const allocator_builder&) = delete;

    auto add_quick_pool() && {
        auto a = adopt_quick_pool(rm, alloc);
        using U = decltype(a);
        return allocator_builder<T, U>{rm, a};
    }

    auto add_thread_safety() && {
        auto a = adopt_thread_safety(rm, alloc);
        using U = decltype(a);
        return allocator_builder<T, U>{rm, a};
    }

    auto build() && {
        return umpire::TypedAllocator<T>{alloc};
    }

    auto build_wrapped() && {
        return u_allocator<T>{umpire::TypedAllocator<T>{alloc}};
    }

  private:
    template<typename U, typename B>
    friend class allocator_builder;

    allocator_builder(umpire::ResourceManager& rm, A a)
    : rm{rm}
    , alloc{a}
    {}
};

template<typename T>
auto builder() { return allocator_builder<T>{}; }

template<typename T>
auto make_test_allocator() {
    return builder<T>()
        .add_quick_pool()
        .add_thread_safety()
        .build();
}

inline constexpr const char* allocator_name = "tspu_malloc";

} // namespace alloc_test

