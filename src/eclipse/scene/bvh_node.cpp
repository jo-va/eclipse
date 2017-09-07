#include "eclipse/scene/bvh_node.h"

#include <ostream>
#include <istream>

namespace eclipse { namespace bvh {

std::istream& operator>>(std::istream& is, Node& n)
{
    is >> n.bbox;
    is >> n.left_data;
    is >> n.right_data;
    return is;
}

std::ostream& operator<<(std::ostream& os, const Node& n)
{
    os << n.bbox;
    os << n.left_data;
    os << n.right_data;
    return os;
}

} } // namespace eclipse::bvh
