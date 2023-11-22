#pragma once

#include <cstdlib>
#include <new>
#include <limits>
#include <memory>
#include <memory_resource>
#include "./mallocator.hpp"

namespace alloc_test {

template<class T>
class pmr_allocator : public std::pmr::polymorphic_allocator<T> {
  public:
    using base = std::pmr::polymorphic_allocator<T>;
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
        using other = pmr_allocator<U>;
    };

    pmr_allocator(base b) : base{b} {}
    pmr_allocator(const pmr_allocator&) = default;
    pmr_allocator(pmr_allocator&&) = default;
    pmr_allocator& operator=(const pmr_allocator&) = default;
    pmr_allocator& operator=(pmr_allocator&&) = default;

    inline friend bool operator==(const pmr_allocator& lhs, const pmr_allocator& rhs) noexcept {
        return static_cast<const base&>(lhs) == static_cast<const base&>(rhs);
    }

    inline friend bool operator!=(const pmr_allocator& lhs, const pmr_allocator& rhs) noexcept {
        return !(lhs==rhs);
    }

    pmr_allocator select_on_container_copy_construction() {
        return *this;
    }
};

template<typename R>
auto adopt_monotonic_pool(const R& r = std::pmr::get_default_resource()) {
    const std::uint32_t buffer_size = 1 << 23;
    auto buffer = std::make_unique<char[]>(buffer_size);
    std::shared_ptr<std::pmr::monotonic_buffer_resource> bufferResource = std::make_shared<std::pmr::monotonic_buffer_resource>(buffer.get(), buffer_size, &r);
    // std::pmr::monotonic_buffer_resource bufferResource(buffer.get(), buffer_size, r);
    return bufferResource;
}

template<typename R>
auto adopt_thread_safe_pool(const R& r = std::pmr::get_default_resource()) {
    std::shared_ptr<std::pmr::synchronized_pool_resource> poolResource = std::make_shared<std::pmr::synchronized_pool_resource>(&r);
    // std::pmr::synchronized_pool_resource poolResource(r);
    return poolResource;
}

// template<typename T, typename A = std::pmr::polymorphic_allocator<T>>
template<typename T, typename R = decltype(std::pmr::get_default_resource())>
struct allocator_builder {

    // A alloc;
    // R resource;

    allocator_builder() {}

    allocator_builder(const allocator_builder&) = delete;
    allocator_builder& operator=(const allocator_builder&) = delete;

    // auto add_monotonic_pool() && {
    //     auto r = adopt_monotonic_pool(alloc.resource());
    //     using U = decltype(*r);
    //     return allocator_builder<T, U>{*r};
    // }

    // auto add_thread_safe_pool() && {
    //     auto r = adopt_thread_safe_pool(resource);
    //     using U = decltype(*r);
    //     return allocator_builder<T, U>{*r};
    // }

    // auto build() && {
    //     return std::pmr::polymorphic_allocator<T>(&resource);
    // }

    auto build() && {
        const std::uint32_t buffer_size = 1 << 23;
        auto buffer = std::make_unique<char[]>(buffer_size);
        std::pmr::monotonic_buffer_resource bufferResource(buffer.get(), buffer_size);
        std::pmr::synchronized_pool_resource poolResource(&bufferResource);
        return std::pmr::polymorphic_allocator<T>(&poolResource);
    }

    // auto build_wrapped() && {
    //     return pmr_allocator<T>{umpire::TypedAllocator<T>{alloc}};
    // }

  private:
    template<typename U, typename B>
    friend class allocator_builder;

    // allocator_builder(A a) : alloc{a} {}
    // allocator_builder(R r) : resource{r} {}
};

template<typename T>
auto builder() { return allocator_builder<T>{}; }

template<typename T>
auto make_test_allocator() {
    return builder<T>()
        // .add_monotonic_pool()
        // .add_thread_safe_pool()
        .build();
}

inline constexpr const char* allocator_name = "pmr_malloc";

} // namespace alloc_test
