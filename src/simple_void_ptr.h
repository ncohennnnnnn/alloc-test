#pragma once

#include <cstddef>
#include <utility>
#include <type_traits>

template<typename MemberFunctionPtr>
struct pmfc;

template<typename R, typename T, typename... Args>
struct pmfc<R (T::*)(Args...)>
{
    typedef R (T::*pfm)(Args...);
    T*  m_ptr;
    pfm m_pmf;
    template<typename... Wargs>
    R operator()(Wargs&&... args) const
    {
        return (m_ptr->*m_pmf)(std::forward<Wargs>(args)...);
    }
};

template<typename R, typename T, typename... Args>
struct pmfc<R (T::*)(Args...) const>
{
    typedef R (T::*pfm)(Args...) const;
    T const* m_ptr;
    pfm      m_pmf;
    template<typename... Wargs>
    R operator()(Wargs&&... args) const
    {
        return (m_ptr->*m_pmf)(std::forward<Wargs>(args)...);
    }
};

template<typename T>
class fancy_ptr;

template<>
class fancy_ptr<void*>
{
  private:
    using this_type = fancy_ptr<void*>;

    void* m_;

  public:
    // constexpr fancy_ptr() noexcept {}
    constexpr fancy_ptr() noexcept : m_{nullptr} {}
    constexpr fancy_ptr(void* ptr) noexcept : m_{ptr} {}
    constexpr fancy_ptr(fancy_ptr const&) noexcept = default;
    // constexpr fancy_ptr(std::nullptr_t) noexcept {}
    constexpr fancy_ptr(std::nullptr_t) noexcept : m_{nullptr} {}
    fancy_ptr& operator=(fancy_ptr const&) noexcept = default;
    fancy_ptr& operator=(std::nullptr_t) noexcept
    {
        m_ = nullptr;
        return *this;
    }
    template<typename U>
    constexpr fancy_ptr& operator=(fancy_ptr<U*> const& ptr) noexcept;

    constexpr friend bool operator==(fancy_ptr a, fancy_ptr b) noexcept
    {
        return (a.m_ == b.m_);
    }
    constexpr friend bool operator!=(fancy_ptr a, fancy_ptr b) noexcept
    {
        return (a.m_ != b.m_);
    }

    constexpr void* get() const noexcept { return m_; }

    constexpr operator bool() const noexcept { return (bool)m_; }

    template<typename U>
    constexpr explicit operator fancy_ptr<U*>() const noexcept;
};

template<typename T>
class fancy_ptr<T*>
{
  private:
    using this_type = fancy_ptr<T*>;
    using void_ptr_t = fancy_ptr<void*>;
    // using const_void_ptr_t = fancy_ptr<void const*>;

  private:
    T* m_;

  public: // iteator typedefs
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::random_access_iterator_tag;

  public:
    constexpr fancy_ptr() noexcept = default;
    constexpr fancy_ptr(T* ptr) noexcept: m_{ptr} {}
    constexpr fancy_ptr(fancy_ptr const&) noexcept = default;
    constexpr fancy_ptr(std::nullptr_t) noexcept: m_{nullptr} {}
    fancy_ptr& operator=(fancy_ptr const&) noexcept = default;
    fancy_ptr& operator=(std::nullptr_t) noexcept
    {
        m_ = nullptr;
        return *this;
    }

    constexpr friend bool operator==(fancy_ptr a, fancy_ptr b) noexcept { return (a.m_ == b.m_); }
    constexpr friend bool operator!=(fancy_ptr a, fancy_ptr b) noexcept { return (a.m_ != b.m_); }

  public:
    reference operator*() const noexcept { return *m_; }
    pointer   operator->() const noexcept { return m_; }
    pointer   get() const noexcept { return m_; }

    // support for pointer to member function
    template<typename R, typename U, typename... Args>
    typename std::enable_if<std::is_same<U, T>::value && std::is_class<U>::value,
        const pmfc<R (U::*)(Args...)>>::type
    operator->*(R (U::*pmf)(Args...)) const noexcept
    {
        return {get(), pmf};
    }

    // support for pointer to const member function
    template<typename R, typename U, typename... Args>
    typename std::enable_if<std::is_same<U, T>::value && std::is_class<U>::value,
        const pmfc<R (U::*)(Args...) const>>::type
    operator->*(R (U::*pmf)(Args...) const) const noexcept
    {
        return {get(), pmf};
    }

    // support for pointer to member
    template<typename M, typename U>
    typename std::enable_if<std::is_same<U, T>::value && std::is_class<U>::value, M&>::type
    operator->*(M U::*pm) const noexcept
    {
        return get()->*pm;
    }

    constexpr explicit operator void_ptr_t() const noexcept { return m_; }
    // constexpr explicit operator const_void_ptr_t() const noexcept { return m_; }
    // needed for std::allocator_traits::construct
    constexpr explicit operator void*() const noexcept { return m_; }
    constexpr          operator bool() const noexcept { return (bool)m_; }


  public: // iterator functions
    this_type& operator++() noexcept
    {
        ++m_;
        return *this;
    }

    this_type operator++(int) noexcept
    {
        auto tmp = *this;
        ++m_;
        return tmp;
    }

    this_type& operator+=(std::ptrdiff_t n) noexcept
    {
        m_+=n;
        return *this;
    }

    friend this_type operator+(this_type a, std::size_t n) noexcept { return (a += n); }

    this_type& operator--() noexcept
    {
        --m_;
        return *this;
    }

    this_type operator--(int) noexcept
    {
        auto tmp = *this;
        --m_;
        return tmp;
    }

    this_type& operator-=(std::ptrdiff_t n) noexcept
    {
        m_-=n;
        return *this;
    }

    friend this_type operator-(this_type a, std::size_t n) noexcept { return (a -= n); }

    friend difference_type operator-(this_type const& a, this_type const& b) noexcept
    {
        return (a.get() - b.get());
    }

    reference& operator[](std::size_t n) const noexcept { return m_[n]; }
};


template<typename U>
constexpr fancy_ptr<void*>& fancy_ptr<void*>::operator=(fancy_ptr<U*> const& ptr) noexcept
{
    m_ = (void*)ptr.m_;
    return *this;
}

template<class U>
constexpr fancy_ptr<void*>::operator fancy_ptr<U*>() const noexcept
{
    return {(U*)m_};
}

template<typename T>
class allocator
{
  public:
    using this_type = allocator<T>;
    using value_type = T;
    using pointer = fancy_ptr<T*>;
    using const_pointer = fancy_ptr<const T*>;
    using void_pointer = fancy_ptr<void*>;
    //using const_void_pointer = fancy_ptr<const void*>;
    using difference_type = typename pointer::difference_type;
    using size_type = std::size_t;

    template<typename U>
    struct other_alloc
    {
        using other = allocator<U>;
    };

    template<typename U>
    using rebind = other_alloc<U>;


  public:
    allocator() = default;

    pointer allocate(size_type n)
    {   
        pointer ptr((T*)std::malloc(n*sizeof(T)));
        return {ptr};
    }

    void deallocate(pointer const& p, size_type)
    {
        std::free(p.get());
    }

    //construct: use default std::allocator_traits implementation
    //destroy:   use default std::allocator_traits implementation
    //max_size:  use default std::allocator_traits implementation
};
