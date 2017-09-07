#include "eclipse/math/vec2.h"

#include <istream>
#include <ostream>

namespace eclipse {

std::istream& operator>>(std::istream& is, Vec2& v)
{
    is >> v.x;
    is >> v.y;
    return is;
}

std::ostream& operator<<(std::ostream& os, const Vec2& v)
{
    os << v.x;
    os << v.y;
    return os;
}

} // namespace eclipse
