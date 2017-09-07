#pragma once

#include "eclipse/prerequisites.h"
#include "eclipse/math/math.h"
#include "eclipse/math/vec3.h"

#include <istream>
#include <ostream>

namespace eclipse {

class BBox
{
public:
    BBox()
    {
        pmin = Vec3(pos_inf, pos_inf, pos_inf);
        pmax = Vec3(neg_inf, neg_inf, neg_inf);
    }

    BBox(const Vec3& p) : pmin(p), pmax(p) { }

    BBox(const Vec3& p1, const Vec3& p2)
    {
        pmin = min(p1, p2);
        pmax = max(p1, p2);
    }

    BBox(const Vec3& p1, const Vec3& p2, const Vec3& p3)
    {
        pmin = min(p1, min(p2, p3));
        pmax = max(p1, max(p2, p3));
    }

    inline BBox& merge(const BBox& o)
    {
        pmin = min(pmin, o.pmin);
        pmax = max(pmax, o.pmax);
        return *this;
    }

    inline BBox& merge(const Vec3& p)
    {
        pmin = min(pmin, p);
        pmax = max(pmax, p);
        return *this;
    }

    inline Vec3 get_centroid()
    {
        return (pmin + pmax) * 0.5f;
    }

    Vec3 pmin, pmax;
};

inline BBox merge(const BBox& b1, const BBox& b2)
{
    return BBox(min(b1.pmin, b2.pmin), max(b1.pmax, b2.pmax));
}

std::istream& operator>>(std::istream& is, BBox& bbox);
std::ostream& operator<<(std::ostream& os, const BBox& bbox);

} // namespace eclipse
