#pragma once

#include "eclipse/math/vec3.h"
#include "eclipse/math/vec4.h"
#include "eclipse/math/mat4.h"
#include "eclipse/math/bbox.h"

namespace eclipse {

class Transform;

class InvTransform
{
public:
    const Transform& ref;

protected:
    InvTransform(const Transform& t) : ref(t) { }
    friend InvTransform inverse(const Transform& t);
};

class Transform
{
public:
    Transform() { }
    explicit Transform(const Mat4& m) : m(m), inv(inverse(m)) { }
    Transform(const Mat4& m, const Mat4& inv) : m(m), inv(inv) { }
    Transform(const Transform& t) : m(t.m), inv(t.inv) { }

    Mat4 get_mat4() const { return m; }

    Transform operator*(const Transform& t) const
    {
        return Transform(m * t.m, t.inv * inv);
    }

    Transform operator/(const Transform& t) const
    {
        return Transform(m * t.inv, t.m * inv);
    }

    Mat4 m;
    Mat4 inv;
};

inline InvTransform inverse(const Transform& t)
{
    return InvTransform(t);
}

inline const Transform& inverse(const InvTransform& t)
{
    return t.ref;
}

inline Vec3 transform_point(const Transform& t, const Vec3& p)
{
    Vec4 res = t.m * Vec4(p, 1.0f);
    if (res.w != 1.0f)
        return Vec3(res.x / res.w, res.y / res.w, res.z / res.w);
    return Vec3(res.x, res.y, res.z);
}

inline Vec3 transform_point(const InvTransform& t, const Vec3& p)
{
    Vec4 res = t.ref.inv * Vec4(p, 1.0f);
    if (res.w != 1.0f)
        return Vec3(res.x / res.w, res.y / res.w, res.z / res.w);
    return Vec3(res.x, res.y, res.z);
}

inline Vec3 transform_vector(const Transform& t, const Vec3& v)
{
    Vec4 res = t.m * Vec4(v, 0.0f);
    return Vec3(res.x, res.y, res.z);
}

inline Vec3 transform_vector(const InvTransform& t, const Vec3& v)
{
    Vec4 res = t.ref.inv * Vec4(v, 0.0f);
    return Vec3(res.x, res.y, res.z);
}

inline Vec3 transform_normal(const Transform& t, const Vec3& n)
{
    Vec4 res = transpose(t.inv) * Vec4(n, 0.0f);
    return Vec3(res.x, res.y, res.z);
}

inline Vec3 transform_normal(const InvTransform& t, const Vec3& n)
{
    Vec4 res = transpose(t.ref.m) * Vec4(n, 0.0f);
    return Vec3(res.x, res.y, res.z);
}

inline BBox transform_bbox(const Transform& t, const BBox& b)
{
    BBox res(transform_point(t, b.pmin));
    res = merge(res, transform_point(t, Vec3(b.pmax.x, b.pmin.y, b.pmin.z)));
    res = merge(res, transform_point(t, Vec3(b.pmin.x, b.pmax.y, b.pmin.z)));
    res = merge(res, transform_point(t, Vec3(b.pmin.x, b.pmin.y, b.pmax.z)));
    res = merge(res, transform_point(t, Vec3(b.pmin.x, b.pmax.y, b.pmax.z)));
    res = merge(res, transform_point(t, Vec3(b.pmax.x, b.pmax.y, b.pmin.z)));
    res = merge(res, transform_point(t, Vec3(b.pmax.x, b.pmin.y, b.pmax.z)));
    res = merge(res, transform_point(t, Vec3(b.pmax.x, b.pmax.y, b.pmax.z)));
    return res;
}

inline BBox transform_bbox(const InvTransform& t, const BBox& b)
{
    BBox res(transform_point(t, b.pmin));
    res = merge(res, transform_point(t, Vec3(b.pmax.x, b.pmin.y, b.pmin.z)));
    res = merge(res, transform_point(t, Vec3(b.pmin.x, b.pmax.y, b.pmin.z)));
    res = merge(res, transform_point(t, Vec3(b.pmin.x, b.pmin.y, b.pmax.z)));
    res = merge(res, transform_point(t, Vec3(b.pmin.x, b.pmax.y, b.pmax.z)));
    res = merge(res, transform_point(t, Vec3(b.pmax.x, b.pmax.y, b.pmin.z)));
    res = merge(res, transform_point(t, Vec3(b.pmax.x, b.pmin.y, b.pmax.z)));
    res = merge(res, transform_point(t, Vec3(b.pmax.x, b.pmax.y, b.pmax.z)));
    return res;
}

Transform translate(const Vec3& delta);
Transform scale(const Vec3& scale);
Transform rotate_x(float angle);
Transform rotate_y(float angle);
Transform rotate_z(float angle);
Transform rotate(const Vec3& axis, float angle);

} // namespace eclipse
