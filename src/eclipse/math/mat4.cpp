#include "eclipse/math/mat4.h"
#include "eclipse/math/math.h"

#include <cstring>
#include <algorithm>

namespace eclipse {

Mat4 inverse(const Mat4& m)
{
    int indxc[4], indxr[4];
    int ipiv[4] = { 0, 0, 0, 0 };
    float minv[4][4];
    Mat4 temp = m;
    memcpy(minv, &temp.m[0][0], 4 * 4 * sizeof(float));
    for (int i = 0; i < 4; ++i)
    {
        int irow = -1, icol = -1;
        float big = 0.0f;
        // Choose pivot
        for (int j = 0; j < 4; ++j)
        {
            if (ipiv[j] != 1.0f)
            {
                for (int k = 0; k < 4; ++k)
                {
                    if (ipiv[k] == 0.0f)
                    {
                        if (abs(minv[j][k]) >= big)
                        {
                            big = abs(minv[j][k]);
                            irow = j;
                            icol = k;
                        }
                    }
                    else if (ipiv[k] > 1)
                    {
                        return Mat4();
                    }
                }
            }
        }
        ++ipiv[icol];

        // Swap rows irow and icol for pivot
        if (irow != icol)
        {
            for (int k = 0; k < 4; ++k)
                std::swap(minv[irow][k], minv[icol][k]);
        }
        indxr[i] = irow;
        indxc[i] = icol;
        if (minv[icol][icol] == 0.0f)
            return Mat4();

        // Set m[icol][icol] to one by scaling row icol appropriately
        float pivinv = 1.0f / minv[icol][icol];
        minv[icol][icol] = 1.0f;
        for (int j = 0; j < 4; ++j)
            minv[icol][j] *= pivinv;

        // Subtract this row from others to zero out their columns
        for (int j = 0; j < 4; ++j)
        {
            if (j != icol)
            {
                float save = minv[j][icol];
                minv[j][icol] = 0.0f;
                for (int k = 0; k < 4; ++k)
                    minv[j][k] -= minv[icol][k] * save;
            }
        }
    }

    // Swap columns to reflect permutation
    for (int j = 3; j >= 0; --j)
    {
        if (indxr[j] != indxc[j])
        {
            for (int k = 0; k < 4; ++k)
                std::swap(minv[k][indxr[j]], minv[k][indxc[j]]);
        }
    }

    Mat4 result;
    std::memcpy(&result.m[0][0], minv, 4 * 4 * sizeof(float));
    return result;
}

} // namespace foundation
