#pragma once

#include "eclipse/prerequisites.h"
#include "eclipse/scene/mat_expr.h"

#include <cstdint>

namespace eclipse {

namespace defaults {

constexpr float Roughness        = 0.1;
constexpr float Reflectance[3]   = { 0.2, 0.2, 0.2 };
constexpr float Specularity[3]   = { 1.0, 1.0, 1.0 };
constexpr float Transmittance[3] = { 1.0, 1.0, 1.0 };
constexpr float Radiance[3]      = { 1.0, 1.0, 1.0 };
constexpr float RadianceScaler   = 1.0;
constexpr char  IntIOR[]         = "Glass";
constexpr char  ExtIOR[]         = "Air";

} // namespace defaults

struct MaterialNode
{
    enum NodeType
    {
        DIFFUSE = 0,
        CONDUCTOR,
        ROUGH_CONDUCTOR,
        DIELECTRIC,
        ROUGH_DIELECTRIC,
        EMISSIVE,
        MIX,
        MIXMAP,
        BUMPMAP,
        NORMALMAP,
        DISPERSE
    };

    enum ParamType
    {
        REFLECTANCE,
        SPECULARITY,
        TRANSMITTANCE,
        RADIANCE,
        INT_IOR,
        EXT_IOR,
        SCALER,
        ROUGHNESS,
        WEIGHT
    };

    uint8_t data[64];

    MaterialNode();

    void set_type(uint32_t type);

    void set_vec3(ParamType param_type, const Vec3& v);
    void set_float(ParamType param_type, float v);
    void set_texture(ParamType param_type, int32_t texture);

    void set_left_child(int32_t left);
    void set_right_child(int32_t right);
} __packed;


} // namespace eclipse
