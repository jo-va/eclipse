#pragma once

#include "eclipse/prerequisites.h"
#include "eclipse/math/math.h"

#include <string>

namespace eclipse {

class Vec3
{
public:
    __forceinline Vec3(float x = 0.0f, float y = 0.0f, float z = 0.0f)
        : x(x), y(y), z(z)
    {
    }

    __forceinline Vec3(const Vec3& o)
        : x(o.x), y(o.y), z(o.z)
    {
    }

    __forceinline const float& operator[](const size_t axis) const { return (&x)[axis]; }
    __forceinline       float& operator[](const size_t axis)       { return (&x)[axis]; }

    float x, y, z;
};

__forceinline Vec3 operator-(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

__forceinline Vec3 operator+(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

__forceinline Vec3 operator*(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

__forceinline Vec3 operator*(const Vec3& v, const float f)
{
    return Vec3(v.x * f, v.y * f, v.z * f);
}

__forceinline Vec3 min(const Vec3& a, const Vec3& b)
{
    return Vec3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}

__forceinline Vec3 max(const Vec3& a, const Vec3& b)
{
    return Vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}

__forceinline Vec3 sqrt(const Vec3& a)
{
    return Vec3(sqrt(a.x), sqrt(a.y), sqrt(a.z));
}

__forceinline Vec3 rsqrt(const Vec3& a)
{
    return Vec3(rsqrt(a.x), rsqrt(a.y), rsqrt(a.z));
}

__forceinline Vec3 cross(const Vec3& a, const Vec3& b)
{
    return Vec3(msub(a.y, b.z, a.z * b.y), msub(a.z, b.x, a.x * b.z), msub(a.x, b.y, a.y * b.x));
}

__forceinline float dot(const Vec3& a, const Vec3& b)
{
    return madd(a.x, b.x, madd(a.y, b.y, a.z * b.z));
}

__forceinline Vec3 normalize(const Vec3& v)
{
    return v * rsqrt(dot(v, v));
}

__forceinline float length(const Vec3& v)
{
    return sqrt(dot(v, v));
}

__forceinline float max_component(const Vec3& v)
{
    float out = v.x;
    if (v.y > v.x) out = v.y;
    if (v.z > v.y) out = v.z;
    return out;
}

std::string to_string(const Vec3& v);

} // namespace eclipse
