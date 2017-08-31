#pragma once

#include "eclipse/prerequisites.h"
#include "eclipse/math/math.h"

namespace eclipse {

class Vec2
{
public:
    __forceinline Vec2(float x = 0.0f, float y = 0.0f)
        : x(x), y(y)
    {
    }

    __forceinline Vec2(const Vec2& o)
        : x(o.x), y(o.y)
    {
    }

    __forceinline const float& operator[](const size_t axis) const { return (&x)[axis]; }
    __forceinline       float& operator[](const size_t axis)       { return (&x)[axis]; }

    float x, y;
};

__forceinline Vec2 operator-(const Vec2& a, const Vec2& b)
{
    return Vec2(a.x - b.x, a.y - b.y);
}

__forceinline Vec2 operator+(const Vec2& a, const Vec2& b)
{
    return Vec2(a.x + b.x, a.y + b.y);
}

__forceinline Vec2 operator*(const Vec2& a, const Vec2& b)
{
    return Vec2(a.x * b.x, a.y * b.y);
}

__forceinline Vec2 operator*(const Vec2& v, const float f)
{
    return Vec2(v.x * f, v.y * f);
}

__forceinline Vec2 min(const Vec2& a, const Vec2& b)
{
    return Vec2(min(a.x, b.x), min(a.y, b.y));
}

__forceinline Vec2 max(const Vec2& a, const Vec2& b)
{
    return Vec2(max(a.x, b.x), max(a.y, b.y));
}

} // namespace eclipse
