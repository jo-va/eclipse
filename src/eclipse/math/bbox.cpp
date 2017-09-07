#include "eclipse/math/bbox.h"

#include <istream>
#include <ostream>

namespace eclipse {

std::istream& operator>>(std::istream& is, BBox& bbox)
{
    is >> bbox.pmin;
    is >> bbox.pmax;
    return is;
}

std::ostream& operator<<(std::ostream& os, const BBox& bbox)
{
    os << bbox.pmin;
    os << bbox.pmax;
    return os;
}

} // namespace eclipse
