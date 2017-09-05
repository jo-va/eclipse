#pragma once

#include "eclipse/prerequisites.h"
#include "eclipse/math/vec4.h"

#include <cstring>
#include <algorithm>
#include <cstring>

namespace eclipse {

struct __packed Mat4
{
    float m[4][4];

    inline Mat4(float m00 = 1.0f, float m01 = 0.0f, float m02 = 0.0f, float m03 = 0.0f,
                float m10 = 0.0f, float m11 = 1.0f, float m12 = 0.0f, float m13 = 0.0f,
                float m20 = 0.0f, float m21 = 0.0f, float m22 = 1.0f, float m23 = 0.0f,
                float m30 = 0.0f, float m31 = 0.0f, float m32 = 0.0f, float m33 = 1.0f)
    {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
        m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
    }

    inline Mat4(const Mat4& o)
    {
        std::memcpy(m, o.m, 4 * 4 * sizeof(float));
    }

    inline Mat4(float values[4][4])
    {
        std::memcpy(m, values, 4 * 4 * sizeof(float));
    }

    inline Mat4& operator=(const Mat4& o)
    {
        std::memcpy(m, o.m, 4 * 4 * sizeof(float));
        return *this;
    }

    inline Mat4 operator-() const;
    inline float trace() const { return m[0][0] + m[1][1] + m[2][2] + m[3][3]; }

    inline Mat4& operator+=(const Mat4& o);
    inline Mat4& operator-=(const Mat4& o);
    inline Mat4& operator*=(const Mat4& o);
    inline Mat4& operator*=(float v);
};

inline Mat4 Mat4::operator-() const
{
    Mat4 res = *this;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            res.m[i][j] = -m[i][j];
    return res;
}

inline Mat4 transpose(const Mat4& m)
{
    Mat4 res;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            res.m[j][i] = m.m[i][j];
    return res;
}

inline Mat4& Mat4::operator+=(const Mat4& o)
{
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            m[i][j] += o.m[i][j];
    return *this;
}

inline Mat4& Mat4::operator-=(const Mat4& o)
{
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            m[i][j] -= o.m[i][j];
    return *this;
}

inline Mat4& Mat4::operator*=(const Mat4& o)
{
    Mat4 temp;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            temp.m[i][j] = 0.0f;
            for (int k = 0; k < 4; ++k)
                temp.m[i][j] += m[i][k] * o.m[k][j];
        }
    }
    *this = temp;
    return *this;
}

inline Mat4& Mat4::operator*=(float v)
{
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            m[i][j] *= v;
    return *this;
}

inline Mat4 operator+(const Mat4& m1, const Mat4& m2)
{
    Mat4 res = m1;
    return res += m2;
}

inline Mat4 operator-(const Mat4& m1, const Mat4& m2)
{
    Mat4 res = m1;
    return res -= m2;
}

inline Mat4 operator*(const Mat4& m1, const Mat4& m2)
{
    Mat4 res;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            res.m[i][j] = 0.0f;
            for (int k = 0; k < 4; ++k)
                res.m[i][j] += m1.m[i][k] * m2.m[k][j];
        }
    }
    return res;
}

inline Mat4 operator*(const Mat4& m, float v)
{
    Mat4 res = m;
    return res *= v;
}

inline Mat4 operator*(float v, const Mat4& m)
{
    Mat4 res = m;
    return res *= v;
}

inline Vec4 operator*(const Mat4& m, const Vec4& v)
{
    Vec4 res;
    for (int i = 0; i < 3; ++i)
    {
        res[i] = 0.0f;
        for (int j = 0; j < 3; ++j)
            res[i] += m.m[i][j] * v[j];
    }
    return res;
}

Mat4 inverse(const Mat4& m);

} // namespace eclipse
