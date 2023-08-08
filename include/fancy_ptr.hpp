#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <memory>

namespace alloc_test {

namespace impl {

template<typename MemberFunctionPtr>
struct pmfc;

template<typename R, typename T, typename... Args>
struct pmfc<R (T::*)(Args...)> {
    typedef R (T::*pfm)(Args...);
    T* m_ptr;
    pfm m_pmf;
    template<typename... Wargs>
    R operator()(Wargs&&... args) const {
        return (m_ptr->*m_pmf)(std::forward<Wargs>(args)...);
    }
};

template<typename R, typename T, typename... Args>
struct pmfc<R (T::*)(Args...) const> {
    typedef R (T::*pfm)(Args...) const;
    T const* m_ptr;
    pfm m_pmf;
    template<typename... Wargs>
    R operator()(Wargs&&... args) const {
        return (m_ptr->*m_pmf)(std::forward<Wargs>(args)...);
    }
};

} // namespace impl

template<typename T>
class fancy_ptr;

template<typename T>
class fancy_ptr<T*> {};

template<>
class fancy_ptr<void> {
  private:
    void* m_ = nullptr;

    template<typename T>
    friend class fancy_ptr;

  public: // ctors
    constexpr fancy_ptr() noexcept {}
    constexpr fancy_ptr(void* p) noexcept : m_{p} {}
    constexpr fancy_ptr(fancy_ptr const&) noexcept = default;
    constexpr fancy_ptr(std::nullptr_t) noexcept {}

  public: // assignment
    fancy_ptr& operator=(fancy_ptr const&) noexcept = default;
    fancy_ptr& operator=(std::nullptr_t) noexcept {
        m_ = nullptr;
        return *this;
    }

  public: // conversion
    template<typename T, typename = std::enable_if_t<!std::is_same_v<void,T> && !std::is_const_v<T>>>
    constexpr fancy_ptr& operator=(fancy_ptr<T> const& ptr) noexcept;

    template<typename T>
    constexpr explicit operator fancy_ptr<T>() const noexcept;

    constexpr operator bool() const noexcept { return m_; }

  public: // comparision
    constexpr friend bool operator==(fancy_ptr a, fancy_ptr b) noexcept { return (a.m_ == b.m_); }
    constexpr friend bool operator!=(fancy_ptr a, fancy_ptr b) noexcept { return (a.m_ != b.m_); }

  public: // get raw pointer
    constexpr void* get() const noexcept { return m_; }
};
    
//using void_pointer = fancy_ptr<void>;

template<>
class fancy_ptr<void const> {
  private:
    void const * m_ = nullptr;

    template<typename T>
    friend class fancy_ptr;

  public: // ctors
    constexpr fancy_ptr() noexcept {}
    constexpr fancy_ptr(void const* p) noexcept : m_{p} {}
    constexpr fancy_ptr(std::nullptr_t) noexcept {}
    constexpr fancy_ptr(fancy_ptr const&) noexcept = default;
    constexpr fancy_ptr(fancy_ptr<void> const& ptr) noexcept : m_{ptr.m_} {}

  public: // assignment
    fancy_ptr& operator=(fancy_ptr const&) noexcept = default;
    fancy_ptr& operator=(std::nullptr_t) noexcept {
        m_ = nullptr;
        return *this;
    }

  public: // conversion
    template<typename T, typename = std::enable_if_t<!std::is_same_v<void const,T>>>
    constexpr fancy_ptr& operator=(fancy_ptr<T> const& ptr) noexcept;

    template<typename T, typename = std::enable_if_t<std::is_const_v<T>>>
    constexpr explicit operator fancy_ptr<T>() const noexcept;

    constexpr operator bool() const noexcept { return m_; }

  public: // comparison
    constexpr friend bool operator==(fancy_ptr a, fancy_ptr b) noexcept { return (a.m_ == b.m_); }
    constexpr friend bool operator!=(fancy_ptr a, fancy_ptr b) noexcept { return (a.m_ != b.m_); }

  public: // get raw pointer
    constexpr void const* get() const noexcept { return m_; }
};

//using const_void_pointer = fancy_ptr<void const>;

template<typename T>
class fancy_ptr {
  private:
    T* m_ = nullptr;

    template<typename U>
    friend class fancy_ptr;

  public: // iteator typedefs
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::random_access_iterator_tag;

  public: // ctors
    constexpr fancy_ptr() noexcept = default;
    constexpr fancy_ptr(T* p) noexcept : m_{p} {}
    constexpr fancy_ptr(fancy_ptr const&) noexcept = default;
    constexpr fancy_ptr(std::nullptr_t) noexcept {}

  public: // assignment
    fancy_ptr& operator=(fancy_ptr const&) noexcept = default;
    fancy_ptr& operator=(std::nullptr_t) noexcept {
        m_ = nullptr;
        return *this;
    }

  public: // conversion
    constexpr explicit operator fancy_ptr<void>() const noexcept { return {(void*)m_}; }
    constexpr explicit operator fancy_ptr<void const>() const noexcept { return {(void const*)m_}; }

    // needed for std::allocator_traits::construct
    constexpr explicit operator void*() const noexcept { return m_; }

    constexpr operator bool() const noexcept { return m_; }

  public: // comparison
    constexpr friend bool operator==(fancy_ptr a, fancy_ptr b) noexcept { return (a.m_ == b.m_); }
    constexpr friend bool operator!=(fancy_ptr a, fancy_ptr b) noexcept { return (a.m_ != b.m_); }

  public: // member access operators
    // indirection
    reference operator*() const noexcept { return *m_; }

    // member of pointer
    pointer operator->() const noexcept { return m_; }

    // pointer to member of pointer
    template<typename M, typename U>
    std::enable_if_t<
        std::is_same<U, T>::value && std::is_class<U>::value,
        M&>
    operator->*(M U::*pm) const noexcept { return get()->*pm; }

    // pointer to memberfunction of pointer
    template<typename R, typename U, typename... Args>
    std::enable_if_t<
        std::is_same<U, T>::value && std::is_class<U>::value,
        const impl::pmfc<R (U::*)(Args...)>>
    operator->*(R (U::*pmf)(Args...)) const noexcept { return {get(), pmf}; }

    // pointer to const member function of pointer
    template<typename R, typename U, typename... Args>
    std::enable_if_t<
        std::is_same<U, T>::value && std::is_class<U>::value,
        const impl::pmfc<R (U::*)(Args...) const>>
    operator->*(R (U::*pmf)(Args...) const) const noexcept { return {get(), pmf}; }

  public: // get raw pointer
    pointer get() const noexcept { return m_; }

  public: // iterator functions
    fancy_ptr& operator++() noexcept {
        ++m_;
        return *this;
    }

    fancy_ptr operator++(int) noexcept {
        auto tmp = *this;
        ++m_;
        return tmp;
    }

    fancy_ptr& operator+=(std::ptrdiff_t n) noexcept {
        m_ += n;
        return *this;
    }

    friend fancy_ptr operator+(fancy_ptr a, std::size_t n) noexcept { return (a += n); }

    fancy_ptr& operator--() noexcept {
        --m_;
        return *this;
    }

    fancy_ptr operator--(int) noexcept {
        auto tmp = *this;
        --m_;
        return tmp;
    }

    fancy_ptr& operator-=(std::ptrdiff_t n) noexcept {
        m_ -= n;
        return *this;
    }

    friend fancy_ptr operator-(fancy_ptr a, std::size_t n) noexcept { return (a -= n); }

    friend difference_type operator-(fancy_ptr const& a, fancy_ptr const& b) noexcept { return (a.m_ - b.m_); }

    reference operator[](std::size_t n) const noexcept { return m_[n]; }
};

template<typename T>
class fancy_ptr<T const> {
  private:
    T const* m_ = nullptr;

    template<typename U>
    friend class fancy_ptr;

  public: // iteator typedefs
    using value_type = T const;
    using difference_type = std::ptrdiff_t;
    using pointer = T const *;
    using reference = T const&;
    using iterator_category = std::random_access_iterator_tag;

  public: // ctors
    constexpr fancy_ptr() noexcept = default;
    constexpr fancy_ptr(T const* p) noexcept : m_{p} {}
    constexpr fancy_ptr(fancy_ptr<T> const & ptr) noexcept : m_{ptr.get()} {}
    constexpr fancy_ptr(fancy_ptr const&) noexcept = default;
    constexpr fancy_ptr(std::nullptr_t) noexcept {}

  public: // assignment
    fancy_ptr& operator=(fancy_ptr const&) noexcept = default;
    fancy_ptr& operator=(std::nullptr_t) noexcept {
        m_ = nullptr;
        return *this;
    }

  public: // conversion
    constexpr explicit operator fancy_ptr<void const>() const noexcept { return {(void const*)m_}; }

    constexpr operator bool() const noexcept { return m_; }

  public: // comparison
    constexpr friend bool operator==(fancy_ptr a, fancy_ptr b) noexcept { return (a.m_ == b.m_); }
    constexpr friend bool operator!=(fancy_ptr a, fancy_ptr b) noexcept { return (a.m_ != b.m_); }

  public: // member access operators
    // indirection
    reference operator*() const noexcept { return *m_; }

    // member of pointer
    pointer operator->() const noexcept { return m_; }

    // pointer to member of pointer
    template<typename M, typename U>
    std::enable_if_t<
        std::is_same<U, T>::value && std::is_class<U>::value, 
        M const&>
    operator->*(const M U::*pm) const noexcept { return get()->*pm; }

    // pointer to const member function of pointer
    template<typename R, typename U, typename... Args>
    std::enable_if_t<
        std::is_same<U, T>::value && std::is_class<U>::value,
        const impl::pmfc<R (U::*)(Args...) const>>
    operator->*(R (U::*pmf)(Args...) const) const noexcept { return {get(), pmf}; }

  public: // get raw pointer
    pointer get() const noexcept { return m_; }

  public: // iterator functions
    fancy_ptr& operator++() noexcept {
        ++m_;
        return *this;
    }

    fancy_ptr operator++(int) noexcept {
        auto tmp = *this;
        ++m_;
        return tmp;
    }

    fancy_ptr& operator+=(std::ptrdiff_t n) noexcept {
        m_ += n;
        return *this;
    }

    friend fancy_ptr operator+(fancy_ptr a, std::size_t n) noexcept { return (a += n); }

    fancy_ptr& operator--() noexcept {
        --m_;
        return *this;
    }

    fancy_ptr operator--(int) noexcept {
        auto tmp = *this;
        --m_;
        return tmp;
    }

    fancy_ptr& operator-=(std::ptrdiff_t n) noexcept {
        m_ -= n;
        return *this;
    }

    friend fancy_ptr operator-(fancy_ptr a, std::size_t n) noexcept { return (a -= n); }

    friend difference_type operator-(fancy_ptr const& a, fancy_ptr const& b) noexcept { return (a.m_ - b.m_); }

    reference operator[](std::size_t n) const noexcept { return m_[n]; }
};

// implementation for fancy_ptr<void> member function templates

template<typename T, typename>
constexpr fancy_ptr<void>& fancy_ptr<void>::operator=(fancy_ptr<T> const& ptr) noexcept {
    m_ = ptr.get();
    return *this;
}

template<typename T>
constexpr fancy_ptr<void>::operator fancy_ptr<T>() const noexcept {
    return {(T*)m_};
}

// implementation for fancy_ptr<void const> member function templates

//template<>
template<typename T, typename>
constexpr fancy_ptr<void const>& fancy_ptr<void const>::operator=(fancy_ptr<T> const& ptr) noexcept {
    m_ = ptr.get();
    return *this;
}

template<typename T, typename>
constexpr fancy_ptr<void const>::operator fancy_ptr<T>() const noexcept {
    return {(T*)m_};
}

} // namespace alloc_test
