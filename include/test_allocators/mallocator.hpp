#define ALLOC_TEST_MALLOCATOR_DEFINITION \
template<class T>\
struct mallocator {\
    using value_type = T;\
 \
    mallocator() = default;\
\
    template<class U>\
    constexpr mallocator(const mallocator<U>&) noexcept {}\
\
    [[nodiscard]] T* allocate(std::size_t n) {\
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))\
            throw std::bad_array_new_length();\
        if (auto p = static_cast<T*>(malloc(n * sizeof(T)))) {\
            return p;\
        }\
        throw std::bad_alloc();\
    }\
 \
    void deallocate(T* p, std::size_t) noexcept {\
        free(p);\
    }\
};\
\
template<class T, class U>\
bool operator==(const mallocator<T>&, const mallocator<U>&) { return true; }\
\
template<class T, class U>\
bool operator!=(const mallocator<T>&, const mallocator<U>&) { return false; }

