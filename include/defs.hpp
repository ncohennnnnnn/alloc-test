#pragma once


#if defined(__GNUC__) || defined(__clang__)
#include <x86intrin.h>
#define FORCE_INLINE __attribute__((always_inline)) inline

#elif defined(_MSC_VER) // Check for Visual C++
#include <intrin.h>
#define FORCE_INLINE __forceinline

#else
#define FORCE_INLINE inline
#endif
