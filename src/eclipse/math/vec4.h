#pragma once

#include "eclipse/prerequisites.h"
#include "eclipse/math/math.h"
#include "eclipse/math/vec3.h"

namespace eclipse {

class Vec4
{
public:
    __forceinline Vec4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f)
        : x(x), y(y), z(z), w(w)
    {
    }

    __forceinline Vec4(const Vec4& o)
        : x(o.x), y(o.y), z(o.z), w(o.w)
    {
    }

    __forceinline Vec4(const Vec3& o)
        : x(o.x), y(o.y), z(o.z), w(0.0f)
    {
    }

    __forceinline const float& operator[](const size_t axis) const { return (&x)[axis]; }
    __forceinline       float& operator[](const size_t axis)       { return (&x)[axis]; }

    float x, y, z, w;
};

} // namespace eclipse
