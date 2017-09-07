#include "eclipse/math/vec4.h"

#include <istream>
#include <ostream>

namespace eclipse {

std::istream& operator>>(std::istream& is, Vec4& v)
{
    is >> v.x;
    is >> v.y;
    is >> v.z;
    is >> v.w;
    return is;
}

std::ostream& operator<<(std::ostream& os, const Vec4& v)
{
    os << v.x;
    os << v.y;
    os << v.z;
    os << v.w;
    return os;
}

} // namespace eclipse
