#pragma once

#include "eclipse/prerequisites.h"

#include <cmath>
#include <limits>
#include <cfloat>
#include <cstdint>

#include <emmintrin.h>
#include <xmmintrin.h>
#include <immintrin.h>

namespace eclipse {

static struct ZeroType
{
    __forceinline operator float   () const { return 0; }
    __forceinline operator double  () const { return 0; }
    __forceinline operator int8_t  () const { return 0; }
    __forceinline operator int16_t () const { return 0; }
    __forceinline operator int32_t () const { return 0; }
    __forceinline operator int64_t () const { return 0; }
    __forceinline operator uint8_t () const { return 0; }
    __forceinline operator uint16_t() const { return 0; }
    __forceinline operator uint32_t() const { return 0; }
    __forceinline operator uint64_t() const { return 0; }
} zero MAYBE_UNUSED;

static struct OneType
{
    __forceinline operator float   () const { return 1; }
    __forceinline operator double  () const { return 1; }
    __forceinline operator int8_t  () const { return 1; }
    __forceinline operator int16_t () const { return 1; }
    __forceinline operator int32_t () const { return 1; }
    __forceinline operator int64_t () const { return 1; }
    __forceinline operator uint8_t () const { return 1; }
    __forceinline operator uint16_t() const { return 1; }
    __forceinline operator uint32_t() const { return 1; }
    __forceinline operator uint64_t() const { return 1; }
} one MAYBE_UNUSED;

static struct NegInfType
{
    __forceinline operator float   () const { return -std::numeric_limits<float>::infinity();  }
    __forceinline operator double  () const { return -std::numeric_limits<double>::infinity(); }
    __forceinline operator int8_t  () const { return  std::numeric_limits<int8_t>::min();      }
    __forceinline operator int16_t () const { return  std::numeric_limits<int16_t>::min();     }
    __forceinline operator int32_t () const { return  std::numeric_limits<int32_t>::min();     }
    __forceinline operator int64_t () const { return  std::numeric_limits<int64_t>::min();     }
    __forceinline operator uint8_t () const { return  std::numeric_limits<uint8_t>::min();     }
    __forceinline operator uint16_t() const { return  std::numeric_limits<uint16_t>::min();    }
    __forceinline operator uint32_t() const { return  std::numeric_limits<uint32_t>::min();    }
    __forceinline operator uint64_t() const { return  std::numeric_limits<uint64_t>::min();    }
} neg_inf MAYBE_UNUSED;

static struct PosInfType
{
    __forceinline operator float   () const { return std::numeric_limits<float>::infinity();  }
    __forceinline operator double  () const { return std::numeric_limits<double>::infinity(); }
    __forceinline operator int8_t  () const { return std::numeric_limits<int8_t>::max();      }
    __forceinline operator int16_t () const { return std::numeric_limits<int16_t>::max();     }
    __forceinline operator int32_t () const { return std::numeric_limits<int32_t>::max();     }
    __forceinline operator int64_t () const { return std::numeric_limits<int64_t>::max();     }
    __forceinline operator uint8_t () const { return std::numeric_limits<uint8_t>::max();     }
    __forceinline operator uint16_t() const { return std::numeric_limits<uint16_t>::max();    }
    __forceinline operator uint32_t() const { return std::numeric_limits<uint32_t>::max();    }
    __forceinline operator uint64_t() const { return std::numeric_limits<uint64_t>::max();    }
} pos_inf MAYBE_UNUSED;

static struct NaNType
{
    __forceinline operator double() const { return std::numeric_limits<double>::quiet_NaN(); }
    __forceinline operator float () const { return std::numeric_limits<float>::quiet_NaN(); }
} NaN MAYBE_UNUSED;

static struct UlpType
{
    __forceinline operator double() const { return std::numeric_limits<double>::epsilon(); }
    __forceinline operator float () const { return std::numeric_limits<float>::epsilon(); }
} ulp MAYBE_UNUSED;

static struct PiType
{
    __forceinline operator double() const { return double(M_PI); }
    __forceinline operator float () const { return  float(M_PI); }
} pi MAYBE_UNUSED;

static struct OneOverPiType
{
    __forceinline operator double() const { return double(M_1_PI); }
    __forceinline operator float () const { return  float(M_1_PI); }
} one_over_pi MAYBE_UNUSED;

__forceinline int cast_f2i(float f) {
    union { float f; int i; } v; v.f = f; return v.i;
}

__forceinline float cast_i2f(int i) {
    union { float f; int i; } v; v.i = i; return v.f;
}

__forceinline float sign(const float x) { return x < 0 ? -1.0f : 1.0f; }
__forceinline float sqr (const float x) { return x * x; }

__forceinline float rcp(const float x)
{
    const __m128 a = _mm_set_ss(x);
    const __m128 r = _mm_rcp_ps(a);
    return _mm_cvtss_f32(_mm_mul_ps(r, _mm_sub_ps(_mm_set_ss(2.0f), _mm_mul_ps(r, a))));
}

__forceinline float rsqrt(const float x)
{
    // Newton Raphson iteration over rsqrt to increase the accuracy ret = y * (1.5 - 0.5 * x * y * y), with y = 1/sqrt(x)
    const __m128 a = _mm_set_ss(x);
    const __m128 r = _mm_rsqrt_ps(a);
    const __m128 c = _mm_add_ps(_mm_mul_ps(_mm_set_ps(1.5f, 1.5f, 1.5f, 1.5f), r),
                                _mm_mul_ps(_mm_mul_ps(_mm_mul_ps(a, _mm_set_ps(-0.5f, -0.5f, -0.5f, -0.5f)), r), _mm_mul_ps(r, r)));
    return _mm_cvtss_f32(c);
}

__forceinline float abs  (const float x) { return std::fabs (x); }
__forceinline float acos (const float x) { return std::acos (x); }
__forceinline float asin (const float x) { return std::asin (x); }
__forceinline float atan (const float x) { return std::atan (x); }
__forceinline float atan2(const float y, const float x) { return std::atan2(y, x); }
__forceinline float cos  (const float x) { return std::cos  (x); }
__forceinline float cosh (const float x) { return std::cosh (x); }
__forceinline float exp  (const float x) { return std::exp  (x); }
__forceinline float fmod (const float x, const float y) { return std::fmod (x, y); }
__forceinline float log  (const float x) { return std::log  (x); }
__forceinline float log10(const float x) { return std::log10(x); }
__forceinline float pow  (const float x, const float y) { return std::pow  (x, y); }
//__forceinline float rcp  (const float x) { return 1.0f / x;      }
//__forceinline float rsqrt(const float x) { return 1.0f / std::sqrt(x); }
__forceinline float sin  (const float x) { return std::sin  (x); }
__forceinline float sinh (const float x) { return std::sinh (x); }
__forceinline float sqrt (const float x) { return std::sqrt (x); }
__forceinline float tan  (const float x) { return std::tan  (x); }
__forceinline float tanh (const float x) { return std::tanh (x); }
__forceinline float floor(const float x) { return std::floor(x); }
__forceinline float ceil (const float x) { return std::ceil (x); }
__forceinline float frac (const float x) { return x-std::floor (x); }
__forceinline float nextafter(float x, float y) { return std::nextafter(x, y); }

__forceinline double abs  (const double x) { return std::fabs (x); }
__forceinline double sign (const double x) { return x < 0 ? -1.0 : 1.0; }
__forceinline double acos (const double x) { return std::acos (x); }
__forceinline double asin (const double x) { return std::asin (x); }
__forceinline double atan (const double x) { return std::atan (x); }
__forceinline double atan2(const double y, const double x) { return std::atan2(y, x); }
__forceinline double cos  (const double x) { return std::cos  (x); }
__forceinline double cosh (const double x) { return std::cosh (x); }
__forceinline double exp  (const double x) { return std::exp  (x); }
__forceinline double fmod (const double x, const double y) { return std::fmod (x, y); }
__forceinline double log  (const double x) { return std::log  (x); }
__forceinline double log10(const double x) { return std::log10(x); }
__forceinline double pow  (const double x, const double y) { return std::pow  (x, y); }
__forceinline double rcp  (const double x) { return 1.0 / x;    }
__forceinline double rsqrt(const double x) { return 1.0 / std::sqrt(x); }
__forceinline double sin  (const double x) { return std::sin  (x); }
__forceinline double sinh (const double x) { return std::sinh (x); }
__forceinline double sqr  (const double x) { return x * x ;        }
__forceinline double sqrt (const double x) { return std::sqrt (x); }
__forceinline double tan  (const double x) { return std::tan  (x); }
__forceinline double tanh (const double x) { return std::tanh (x); }
__forceinline double floor(const double x) { return std::floor(x); }
__forceinline double ceil (const double x) { return std::ceil (x); }
__forceinline double nextafter(double x, double y) { return std::nextafter(x, y); }

__forceinline float madd (const float a, const float b, const float c) { return  a * b + c; }
__forceinline float msub (const float a, const float b, const float c) { return  a * b - c; }
__forceinline float nmadd(const float a, const float b, const float c) { return -a * b + c; }
__forceinline float nmsub(const float a, const float b, const float c) { return -a * b - c; }

__forceinline bool  select(bool s, bool  t , bool f) { return s ? t : f; }
__forceinline int   select(bool s, int   t,   int f) { return s ? t : f; }
__forceinline float select(bool s, float t, float f) { return s ? t : f; }

#if defined(__SSE4_1__)
    __forceinline float mini(float a, float b) {
        const __m128i ai = _mm_castps_si128(_mm_set_ss(a));
        const __m128i bi = _mm_castps_si128(_mm_set_ss(b));
        const __m128i ci = _mm_min_epi32(ai,bi);
        return _mm_cvtss_f32(_mm_castsi128_ps(ci));
    }
#endif

#if defined(__SSE4_1__)
    __forceinline float maxi(float a, float b) {
        const __m128i ai = _mm_castps_si128(_mm_set_ss(a));
        const __m128i bi = _mm_castps_si128(_mm_set_ss(b));
        const __m128i ci = _mm_max_epi32(ai,bi);
        return _mm_cvtss_f32(_mm_castsi128_ps(ci));
    }
#endif

template<typename T> __forceinline T twice(const T& a) { return a+a; }

__forceinline    float min(float    a, float    b) { return a < b ? a : b; }
__forceinline   double min(double   a, double   b) { return a < b ? a : b; }
__forceinline   int8_t min(int8_t   a, int8_t   b) { return a < b ? a : b; }
__forceinline  int16_t min(int16_t  a, int16_t  b) { return a < b ? a : b; }
__forceinline  int32_t min(int32_t  a, int32_t  b) { return a < b ? a : b; }
__forceinline  int64_t min(int64_t  a, int64_t  b) { return a < b ? a : b; }
__forceinline  uint8_t min(uint8_t  a, uint8_t  b) { return a < b ? a : b; }
__forceinline uint16_t min(uint16_t a, uint16_t b) { return a < b ? a : b; }
__forceinline uint32_t min(uint32_t a, uint32_t b) { return a < b ? a : b; }
__forceinline uint64_t min(uint64_t a, uint64_t b) { return a < b ? a : b; }

template<typename T> __forceinline T min(const T& a, const T& b, const T& c) { return min(min(a, b), c); }
template<typename T> __forceinline T min(const T& a, const T& b, const T& c, const T& d) { return min(min(a, b), min(c, d)); }
template<typename T> __forceinline T min(const T& a, const T& b, const T& c, const T& d, const T& e) { return min(min(min(a, b), min(c, d)), e); }

template<typename T> __forceinline T mini(const T& a, const T& b, const T& c) { return mini(mini(a, b), c); }
template<typename T> __forceinline T mini(const T& a, const T& b, const T& c, const T& d) { return mini(mini(a, b), mini(c, d)); }
template<typename T> __forceinline T mini(const T& a, const T& b, const T& c, const T& d, const T& e) { return mini(mini(mini(a, b), mini(c, d)), e); }

__forceinline    float max(float    a, float    b) { return a > b ? a : b; }
__forceinline   double max(double   a, double   b) { return a > b ? a : b; }
__forceinline   int8_t max(int8_t   a, int8_t   b) { return a > b ? a : b; }
__forceinline  int16_t max(int16_t  a, int16_t  b) { return a > b ? a : b; }
__forceinline  int32_t max(int32_t  a, int32_t  b) { return a > b ? a : b; }
__forceinline  int64_t max(int64_t  a, int64_t  b) { return a > b ? a : b; }
__forceinline  uint8_t max(uint8_t  a, uint8_t  b) { return a > b ? a : b; }
__forceinline uint16_t max(uint16_t a, uint16_t b) { return a > b ? a : b; }
__forceinline uint32_t max(uint32_t a, uint32_t b) { return a > b ? a : b; }
__forceinline uint64_t max(uint64_t a, uint64_t b) { return a > b ? a : b; }

template<typename T> __forceinline T max(const T& a, const T& b, const T& c) { return max(max(a, b), c); }
template<typename T> __forceinline T max(const T& a, const T& b, const T& c, const T& d) { return max(max(a, b), max(c, d)); }
template<typename T> __forceinline T max(const T& a, const T& b, const T& c, const T& d, const T& e) { return max(max(max(a, b), max(c, d)), e); }

template<typename T> __forceinline T maxi(const T& a, const T& b, const T& c) { return maxi(maxi(a, b), c); }
template<typename T> __forceinline T maxi(const T& a, const T& b, const T& c, const T& d) { return maxi(maxi(a, b), maxi(c, d)); }
template<typename T> __forceinline T maxi(const T& a, const T& b, const T& c, const T& d, const T& e) { return maxi(maxi(maxi(a, b), maxi(c, d)), e); }

template<typename T> __forceinline T clamp(const T& x, const T& lower = T(zero), const T& upper = T(one)) { return max(min(x, upper), lower); }
template<typename T> __forceinline T clampz(const T& x, const T& upper) { return max(T(zero), min(x, upper)); }

template<typename T> __forceinline T deg2rad(const T& x) { return x * T(1.74532925199432957692e-2f);  }
template<typename T> __forceinline T rad2deg(const T& x) { return x * T(5.72957795130823208768e1f);   }
template<typename T> __forceinline T sin2cos(const T& x) { return sqrt(max(T(zero), T(one) - x * x)); }
template<typename T> __forceinline T cos2sin(const T& x) { return sin2cos(x);                         }

template<typename T> T random() { return T(0); }
template<> __forceinline int      random() { return int(rand()); }
template<> __forceinline uint32_t random() { return uint32_t(rand()) ^ (uint32_t(rand()) << 16); }
template<> __forceinline float    random() { return rand() / float(RAND_MAX); }
template<> __forceinline double   random() { return rand() / double(RAND_MAX); }

template<typename T> __forceinline T lerp(const T& v0, const T& v1, const float t) {
    return madd(T(1.0f - t), v0, t * v1);
}

template<typename T> __forceinline T lerp2(const float x0, const float x1, const float x2, const float x3, const T& u, const T& v) {
    return madd((1.0f - u), madd((1.0f - v), T(x0), v * T(x2)), u * madd((1.0f - v), T(x1), v * T(x3)));
}

template<typename T> __forceinline void xchg(T& a, T& b) { const T tmp = a; a = b; b = tmp; }

} // namespace eclipse
