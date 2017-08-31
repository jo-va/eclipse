#pragma once

#include "eclipse/math/math.h"
#include "eclipse/math/vec3.h"
#include "eclipse/math/mat4.h"

namespace eclipse {

class Quaternion
{
public:
    inline Quaternion() : v(0.0f), w(1.0f) { }
    inline Quaternion(const Quaternion& q) : v(q.v), w(q.w) { }
    inline Quaternion(const Vec3& v, float w) : v(v), w(w) { }

    Quaternion inverse() const { return Quaternion(-v, w); }

    void to_matrix(Mat4& mat4) const;

    Vec3 v;
    float w;
};

inline Quaternion operator+(const Quaternion& q1, const Quaternion& q2)
{
    return Quaternion(q1.v + q2.v, q1.w + q2.w);
}

inline Quaternion operator-(const Quaternion& q1, const Quaternion& q2)
{
    return Quaternion(q1.v - q2.v, q1.w - q2.w);
}

inline Quaternion operator*(float f, const Quaternion& q)
{
    return Quaternion(f * q.v, f * q.w);
}

inline Quaternion operator*(const Quaternion& q1, const Quaternion& q2)
{
    return Quaternion(
            q1.w * q2.v + q2.w * q1.v + cross(q1.v, q2.v),
            q1.w * q2.w - dot(q1.v, q2.v));
}

inline float dot(const Quaternion& q1, const Quaternion& q2)
{
    return q1.w * q2.w + dot(q1.v, q2.v);
}

inline Quaternion normalize(const Quaternion& q)
{
    return (1.0f / sqrt(dot(q, q))) * q;
}

} // namespace eclipse
