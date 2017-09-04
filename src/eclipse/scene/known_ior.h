#pragma once

#include <string>

namespace eclipse { namespace material {

// Get the IOR value of a known material by name
// If the material is found, the result is returned
// otherwise a negative value is returned and the error message is set;
// the material name is case insensitive
float get_known_ior(const std::string& material, std::string& error_msg);

} } // namespace eclipse::material
