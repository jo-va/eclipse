#include "eclipse/math/quaternion.h"

namespace eclipse {

void Quaternion::to_matrix(Mat4& m) const
{
    const float xx = v.x * v.x;
    const float yy = v.y * v.y;
    const float zz = v.z * v.z;
    const float xy = v.x * v.y;
    const float xz = v.x * v.z;
    const float yz = v.y * v.z;
    const float xw = v.x * w;
    const float yw = v.y * w;
    const float zw = v.z * w;

    m.m[0][0] = 1.0f - 2.0f * (yy + zz);
    m.m[1][0] = 2.0f * (xy - zw);
    m.m[2][0] = 2.0f * (xz + yw);
    m.m[0][1] = 2.0f * (xy + zw);
    m.m[1][1] = 1.0f - 2.0f * (xx + zz);
    m.m[2][1] = 2.0f * (yz - xw);
    m.m[0][2] = 2.0f * (xz - yw);
    m.m[1][2] = 2.0f * (yz + xw);
    m.m[2][2] = 1.0f - 2.0f * (xx + yy);

    m.m[0][3] = m.m[1][3] = m.m[2][3] = 0.0f;
    m.m[3][0] = m.m[3][1] = m.m[3][2] = 0.0f;
    m.m[3][3] = 1.0f;
}

} // namespace eclipse
