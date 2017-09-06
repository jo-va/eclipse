#pragma once

#include <string>

namespace eclipse { namespace material {

// Get the IOR value of a known material by name
// If the material is found, the result is returned
// otherwise an exception is thrown
float get_known_ior(const std::string& material);

} } // namespace eclipse::material
