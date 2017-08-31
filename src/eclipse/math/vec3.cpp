#include "eclipse/math/vec3.h"

#include <string>

namespace eclipse {

std::string to_string(const Vec3& v)
{
    return "{" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + " }";
}

} // namespace eclipse
