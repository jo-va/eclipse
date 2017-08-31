#include "eclipse/math/transform.h"
#include "eclipse/math/vec3.h"

namespace eclipse {

Transform translate(const Vec3& delta)
{
    Mat4 m(1, 0, 0, delta.x,
           0, 1, 0, delta.y,
           0, 0, 1, delta.z,
           0, 0, 0, 1);

    Mat4 i(1, 0, 0, -delta.x,
           0, 1, 0, -delta.y,
           0, 0, 1, -delta.z,
           0, 0, 0, 1);

    return Transform(m, i);
}

Transform scale(const Vec3& s)
{
    Mat4 m(s.x, 0, 0, 0,
           0, s.y, 0, 0,
           0, 0, s.z, 0,
           0, 0, 0, 1);

    Mat4 i(1.0f / s.x, 0, 0, 0,
           0, 1.0f / s.y, 0, 0,
           0, 0, 1.0f / s.z, 0,
           0, 0, 0, 1);

    return Transform(m, i);
}

Transform rotate_x(float angle)
{
    const float s = sin(radians(angle));
    const float c = cos(radians(angle));

    Mat4 m(1, 0, 0, 0,
           0, c, -s, 0,
           0, s, c, 0,
           0, 0, 0, 1);

    return Transform(m, transpose(m));
}

Transform rotate_y(float angle)
{
    const float s = sin(radians(angle));
    const float c = cos(radians(angle));

    Mat4 m(c, 0, s, 0,
           0, 1, 0, 0,
           -s, 0, c, 0,
           0, 0, 0, 1);

    return Transform(m, transpose(m));
}

Transform rotate_z(float angle)
{
    const float s = sin(radians(angle));
    const float c = cos(radians(angle));

    Mat4 m(c, -s, 0, 0,
           s, c, 0, 0,
           0, 0, 1, 0,
           0, 0, 0, 1);

    return Transform(m, transpose(m));
}

Transform rotate(const Vec3& axis, float angle)
{
    const Vec3 a = normalize(axis);
    const float s = sin(radians(angle));
    const float c = cos(radians(angle));
    Mat4 m;

    m.m[0][0] = a.x * a.x + (1.0f - a.x * a.x) * c;
	m.m[0][1] = a.x * a.y * (1.0f - c) - a.z * s;
	m.m[0][2] = a.x * a.z * (1.0f - c) + a.y * s;
	m.m[0][3] = 0.0f;

	m.m[1][0] = a.x * a.y * (1.0f - c) + a.z * s;
	m.m[1][1] = a.y * a.y + (1.0f - a.y * a.y) * c;
	m.m[1][2] = a.y * a.z * (1.0f - c) - a.x * s;
	m.m[1][3] = 0.0f;

	m.m[2][0] = a.x * a.z * (1.0f - c) - a.y * s;
	m.m[2][1] = a.y * a.z * (1.0f - c) + a.x * s;
	m.m[2][2] = a.z * a.z + (1.0f - a.z * a.z) * c;
	m.m[2][3] = 0.0f;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	m.m[3][2] = 0.0f;
	m.m[3][3] = 1.0f;

    return Transform(m, transpose(m));
}

} // namespace eclipse
