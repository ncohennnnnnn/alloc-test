#pragma once

#include <fancy_ptr.hpp>

#include <memory>
#include <type_traits>

namespace alloc_test {

template<class Allocator>
class fancy_allocator_adaptor : public Allocator {
  private:
    using base = Allocator;
    using traits = std::allocator_traits<Allocator>;

  public:
    using value_type = typename traits::value_type;
    using T = value_type;
    using pointer = fancy_ptr<T>;
    using const_pointer = fancy_ptr<T const>;
    using void_pointer = fancy_ptr<void>;
    using const_void_pointer = fancy_ptr<void const>;
    using size_type = std::size_t;
    using difference_type = typename std::pointer_traits<pointer>::difference_type;
    using is_always_equal = typename traits::is_always_equal;
    using propagate_on_container_copy_assignment = typename traits::propagate_on_container_copy_assignment;
    using propagate_on_container_move_assignment = typename traits::propagate_on_container_move_assignment;
    using propagate_on_container_swap = typename traits::propagate_on_container_swap;

    template<typename U>
    struct rebind {
        using other = fancy_allocator_adaptor<typename traits::template rebind_alloc<U>>;
    };

    template<typename A>
    static constexpr bool is_rebound_base = std::is_same_v<A,
        typename traits::template rebind_alloc<typename std::allocator_traits<A>::value_type>>;

  private:
    base& upcast() noexcept { return static_cast<base&>(*this); }
    const base& upcast() const noexcept { return static_cast<const base&>(*this); }

  public:
    template<typename...Args>
    fancy_allocator_adaptor(Args&&...args): base{std::forward<Args>(args)...} {}

    template<typename A, typename = std::enable_if_t<is_rebound_base<A>>>
    fancy_allocator_adaptor(const fancy_allocator_adaptor<A>& other): base{static_cast<const A&>(other)} {}

    template<typename A, typename = std::enable_if_t<is_rebound_base<A>>>
    fancy_allocator_adaptor& operator=(const fancy_allocator_adaptor<A>& other) {
        upcast() = static_cast<const A&>(other);
        return *this;
    }

    fancy_allocator_adaptor select_on_container_copy_construction() {
        return {traits::select_on_container_copy_construction(upcast())};
    }

  public:
    [[nodiscard]] pointer allocate(std::size_t n) { return {traits::allocate(upcast(), n)}; }
    [[nodiscard]] pointer allocate(size_type n, const_void_pointer cvp) { return {traits::allocate(upcast(), n, cvp.get())}; }
    void deallocate(pointer p, size_type n) { traits::deallocate(upcast(), p.get(), n); }
    size_type max_size() noexcept { return traits::max_size(upcast()); }

    template<typename T, class... Args>
    void construct(T* p, Args&&... args) { traits::construct(upcast(), p, std::forward<Args>(args)...); }

    template<class... Args>
    void construct(pointer p, Args&&... args) { traits::construct(upcast(), p.get(), std::forward<Args>(args)...); }

    template<typename T>
    void destroy(T* p) { traits::destroy(upcast(), p); }

    void destroy(pointer p) { traits::destroy(upcast(), p.get()); }
};

template<class A, class B>
bool operator==(const fancy_allocator_adaptor<A>& a, const fancy_allocator_adaptor<B>& b) {
    return static_cast<const A&>(a) == static_cast<const B&>(b);
}
 
template<class A, class B>
bool operator!=(const fancy_allocator_adaptor<A>& a, const fancy_allocator_adaptor<B>& b) {
    return static_cast<const A&>(a) != static_cast<const B&>(b);
}

} // namespace alloc_test
