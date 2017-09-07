#include "eclipse/math/vec3.h"

#include <string>
#include <istream>
#include <ostream>

namespace eclipse {

std::string to_string(const Vec3& v)
{
    return "{" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + " }";
}

std::istream& operator>>(std::istream& is, Vec3& v)
{
    is >> v.x;
    is >> v.y;
    is >> v.z;
    return is;
}

std::ostream& operator<<(std::ostream& os, const Vec3& v)
{
    os << v.x;
    os << v.y;
    os << v.z;
    return os;
}

} // namespace eclipse
