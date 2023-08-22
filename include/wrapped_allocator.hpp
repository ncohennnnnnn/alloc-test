#pragma once

#include <defs.hpp>
#include <to_address.hpp>

#include <memory>
#include <type_traits>
#include <utility>
#include <cassert>

namespace alloc_test {

#define ALLOC_TEST_OPTIONAL_MEMBER_FUNCTION_DISPATCH(function_name) \
template<typename A, typename = void>                               \
struct has_ ## function_name {                                      \
    static void function_name(A&){}                                 \
};                                                                  \
template<typename A>                                                \
struct has_ ## function_name<A,                                     \
    std::void_t<decltype(std::declval<A>(). function_name())>> {    \
    static void function_name(A& a){ a. function_name(); }          \
};

ALLOC_TEST_OPTIONAL_MEMBER_FUNCTION_DISPATCH(init)
ALLOC_TEST_OPTIONAL_MEMBER_FUNCTION_DISPATCH(deinit)
ALLOC_TEST_OPTIONAL_MEMBER_FUNCTION_DISPATCH(after_setup_phase)
ALLOC_TEST_OPTIONAL_MEMBER_FUNCTION_DISPATCH(after_main_loop_phase)
ALLOC_TEST_OPTIONAL_MEMBER_FUNCTION_DISPATCH(after_cleanup_phase)

template<typename A>
struct extended_allocator_traits : public has_init<A>, has_deinit<A>,
has_after_setup_phase<A>, has_after_main_loop_phase<A>, has_after_cleanup_phase<A> {};

template<class Allocator>
class wrapped_allocator {
  public:
    using internal_allocator = Allocator;
    using alloc_traits = std::allocator_traits<internal_allocator>;
    using value_type = typename alloc_traits::value_type;
    using pointer = typename alloc_traits::pointer;
    using const_pointer = typename alloc_traits::const_pointer;
    using void_pointer = typename alloc_traits::void_pointer;
    using const_void_pointer = typename alloc_traits::const_void_pointer;
    using diffrence_type = typename alloc_traits::difference_type;
    using size_type = typename alloc_traits::size_type;

    static constexpr bool uses_raw_ptr = std::is_same<pointer, value_type*>::value;

    template<typename T>
    struct rebind {
        using other = wrapped_allocator<typename alloc_traits::template rebind_alloc<T>>;
    };

    internal_allocator m_;

    wrapped_allocator(Allocator a) : m_{a} {}
    wrapped_allocator(const wrapped_allocator&) = default;
    wrapped_allocator(wrapped_allocator&&) = default;
    wrapped_allocator& operator=(const wrapped_allocator&) = default;
    wrapped_allocator& operator=(wrapped_allocator&&) = default;

    inline friend bool operator==(const wrapped_allocator& lhs, const wrapped_allocator& rhs) noexcept {
        return lhs.m_ == rhs.m_;
    }

    inline friend bool operator!=(const wrapped_allocator& lhs, const wrapped_allocator& rhs) noexcept {
        return !(lhs==rhs);
    }

    [[nodiscard]] pointer allocate(size_type n) { return alloc_traits::allocate(m_, n); }

    [[nodiscard]] pointer allocate(size_type n, const_void_pointer cvp) { return alloc_traits::allocate(m_, n, cvp); }

    void deallocate(pointer p, size_type n) { alloc_traits::deallocate(m_, p, n); }

    size_type max_size() noexcept { return alloc_traits::max_size(m_); }

    template<typename T, class... Args>
    std::enable_if_t<!std::is_same_v<T*, pointer>, void>
    construct(T* p, Args&&... args) {
        ::new (static_cast<void*>(p)) T(std::forward<Args>(args)...);
    }

    template<class... Args>
    void construct(pointer p, Args&&... args) {
        ::new (static_cast<void*>(to_address(p))) value_type(std::forward<Args>(args)...);
    }

    template<typename T>
    std::enable_if_t<!std::is_same_v<T*, pointer>, void>
    destroy(T* p) {
        p->~T(); 
    }

    void destroy(pointer p) {
        p->~value_type();
    }

    using propagate_on_container_copy_assignment = typename alloc_traits::propagate_on_container_copy_assignment;
    using propagate_on_container_move_assignment = typename alloc_traits::propagate_on_container_move_assignment;
    using propagate_on_container_swap = typename alloc_traits::propagate_on_container_swap;
    using is_always_equal = std::false_type;

    wrapped_allocator select_on_container_copy_construction() {
        auto new_m = alloc_traits::select_on_container_copy_construction(m_);
        wrapped_allocator tmp(*this);
        tmp.m_ = new_m;
        return tmp;
    }

    internal_allocator get_internal() const { return m_; }

    void init() { extended_allocator_traits<Allocator>::init(m_); }
    void deinit() { extended_allocator_traits<Allocator>::deinit(m_); }
    void after_setup_phase() { extended_allocator_traits<Allocator>::after_setup_phase(m_); }
    void after_main_loop_phase() { extended_allocator_traits<Allocator>::after_main_loop_phase(m_); }
    void after_cleanup_phase() { extended_allocator_traits<Allocator>::after_cleanup_phase(m_); }
};

template<class Allocator>
inline auto wrap_alloc(const Allocator& a) { return wrapped_allocator<Allocator>{a}; }

template<class Allocator>
class fake_allocator : public wrapped_allocator<Allocator>
{
  public:
    using base = wrapped_allocator<Allocator>;
    using internal_allocator = Allocator;
    using alloc_traits = std::allocator_traits<base>;
    using value_type = typename alloc_traits::value_type;
    using pointer = typename alloc_traits::pointer;
    using const_pointer = typename alloc_traits::const_pointer;
    using void_pointer = typename alloc_traits::void_pointer;
    using const_void_pointer = typename alloc_traits::const_void_pointer;
    using diffrence_type = typename alloc_traits::difference_type;
    using size_type = typename alloc_traits::size_type;
    using propagate_on_container_copy_assignment = typename alloc_traits::propagate_on_container_copy_assignment;
    using propagate_on_container_move_assignment = typename alloc_traits::propagate_on_container_move_assignment;
    using propagate_on_container_swap = typename alloc_traits::propagate_on_container_swap;
    using is_always_equal = typename alloc_traits::is_always_equal;

    template<typename T>
    struct rebind {
        using other = fake_allocator<
            typename base::template rebind<T>::other::internal_allocator>;
    };


  private:
    pointer buffer_ = nullptr;
    size_type buffer_size_ = 0u;

    fake_allocator(Allocator a, pointer buffer_, size_type s)
    : base(a), buffer_{buffer_}, buffer_size_{s} {}

    fake_allocator(const base& b, pointer buffer_, size_type s)
    : base(b), buffer_{buffer_}, buffer_size_{s} {}

  public:
    fake_allocator(const fake_allocator&) = default;
    fake_allocator(fake_allocator&&) = default;
    fake_allocator& operator=(const fake_allocator&) = default;
    fake_allocator& operator=(fake_allocator&&) = default;

    inline friend bool operator==(const fake_allocator& lhs, const fake_allocator& rhs) noexcept {
        return (static_cast<const base&>(lhs) == static_cast<const base&>(rhs)) && 
            (lhs.buffer_ == rhs.buffer_);
    }

    inline friend bool operator!=(const fake_allocator& lhs, const fake_allocator& rhs) noexcept {
        return !(lhs==rhs);
    }

    [[nodiscard]] pointer allocate([[maybe_unused]] size_type n) {
        assert(n <= buffer_size_);
        return buffer_;
    }

    [[nodiscard]] pointer allocate(size_type n, const_void_pointer) {
        return this->allocate(n);
    }

    void deallocate(pointer, size_type) {}

    size_type max_size() noexcept { return buffer_size_; }

    fake_allocator select_on_container_copy_construction() {
        return {alloc_traits::select_on_container_copy_construction(static_cast<const base&>(*this)), buffer_, buffer_size_};
    }

  public:
    class buffer_resource {
      private:
        using alloc_traits = std::allocator_traits<Allocator>;

        friend class fake_allocator<Allocator>;

        Allocator a;
        pointer buffer = nullptr;
        size_type size = 0u;

        buffer_resource(Allocator alloc, size_type s)
        : a{alloc}, buffer{alloc_traits::allocate(a, s)}, size{s}
        {
            for (size_type i=0; i<size; ++i) {
                alloc_traits::construct(a, to_address(buffer+i));
            }
        }

      public:
        buffer_resource(const buffer_resource&) = delete;

        buffer_resource(buffer_resource&& other)
        : a{std::move(other.a)}
        , buffer{std::exchange(other.buffer, nullptr)}
        , size{std::exchange(other.size, 0u)}
        {}

        ~buffer_resource() { release(); }

      private:
        void release() {
            if (buffer) {
                for (size_type i=0; i<size; ++i) {
                    alloc_traits::destroy(a, to_address(buffer+i));
                }
                alloc_traits::deallocate(a, buffer, size);
            }
        }
    };

    [[nodiscard]] static std::pair<buffer_resource, fake_allocator> create(Allocator a, size_type s) {
        buffer_resource res(a,s);
        fake_allocator f{res.a, res.buffer, s};
        return {std::move(res), f};
    }

    void after_setup_phase() {}
    void after_main_loop_phase() {}
    void after_cleanup_phase() {}
};

template<class Allocator>
[[nodiscard]] inline auto fake_alloc(const Allocator& a, std::size_t size) {
    return fake_allocator<Allocator>::create(a, size);
}

} // namespace alloc_test
