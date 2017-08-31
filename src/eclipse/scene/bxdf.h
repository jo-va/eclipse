#pragma once

#include <cstdint>
#include <string>

namespace eclipse { namespace material {

enum class BXDF : uint32_t
{
    INVALID = 0,
    EMISSIVE,
    DIFFUSE,
    CONDUCTOR,
    ROUGH_CONDUCTOR,
    DIELECTRIC,
    ROUGH_DIELECTRIC,
    LAST_ENTRY
};

bool is_valid(uint32_t value)
{
    return value > uint32_t(BXDF::INVALID) && value < uint32_t(BXDF::LAST_ENTRY);
}

BXDF bxdf_from_string(const std::string& name)
{
    if (name == "emissive")
        return BXDF::EMISSIVE;
    else if (name == "diffuse")
        return BXDF::DIFFUSE;
    else if (name == "conductor")
        return BXDF::CONDUCTOR;
    else if (name == "roughConductor")
        return BXDF::ROUGH_CONDUCTOR;
    else if (name == "dielectric")
        return BXDF::DIELECTRIC;
    else if (name == "roughDielectric")
        return BXDF::ROUGH_DIELECTRIC;

    return BXDF::INVALID;
}

std::string bxdf_to_string(BXDF bxdf)
{
    switch (bxdf)
    {
        case BXDF::EMISSIVE:         return "emissive";
        case BXDF::DIFFUSE:          return "diffuse";
        case BXDF::CONDUCTOR:        return "conductor";
        case BXDF::ROUGH_CONDUCTOR:  return "roughConductor";
        case BXDF::DIELECTRIC:       return "dielectric";
        case BXDF::ROUGH_DIELECTRIC: return "roughDielectric";
        default:                     return "invalid";
    }
}

} } // namespace eclipse::material
