#pragma once

namespace eclipse {

#define LOG_TIMESTAMP
#define LOG_COLOR

#if !defined(__noinline)
#   define __noinline __attribute__((noinline))
#endif

#if !defined(__forceinline)
#   define __forceinline inline __attribute__((always_inline))
#endif

#if !defined(__aligned)
#   define __aligned(...) __attribute__((aligned(__VA_ARGS__)))
#endif

#if !defined(__packed)
#   define __packed __attribute__((__packed__))
#endif

#if defined(__clang__) || defined(__GNUC__)
#   define MAYBE_UNUSED __attribute__((unused))
#else
#   define MAYBE_UNUSED
#endif

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#   define   likely(expr) (expr)
#   define unlikely(expr) (expr)
#else
#   define   likely(expr) __builtin_expect((bool)(expr), true)
#   define unlikely(expr) __builtin_expect((bool)(expr), false)
#endif

} // namespace eclipse
