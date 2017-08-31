#pragma once

#include "eclipse/prerequisites.h"
#include "eclipse/math/math.h"
#include "eclipse/math/vec3.h"

namespace eclipse {

class Vec4
{
public:
    inline Vec4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f)
        : x(x), y(y), z(z), w(w)
    {
    }

    inline Vec4(const Vec4& o)
        : x(o.x), y(o.y), z(o.z), w(o.w)
    {
    }

    inline Vec4(const Vec3& o, float w)
        : x(o.x), y(o.y), z(o.z), w(w)
    {
    }

    inline const float& operator[](const size_t axis) const { return (&x)[axis]; }
    inline       float& operator[](const size_t axis)       { return (&x)[axis]; }

    float x, y, z, w;
};

} // namespace eclipse
