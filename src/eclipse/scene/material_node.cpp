#include "eclipse/scene/material_node.h"
#include "eclipse/scene/mat_expr.h"
#include "eclipse/math/vec3.h"

#include <string>
#include <cstring>
#include <cstdint>

namespace eclipse {

MaterialNode::MaterialNode()
{
    memset(this, 0, sizeof(*this));
    int32_t* iptr = (int32_t*)data;

    // Set textures to -1
    iptr[1] = -1;
    iptr[2] = -1;
    iptr[3] = -1;
    iptr[15] = -1;
}

void MaterialNode::set_type(uint32_t type)
{
    *reinterpret_cast<uint32_t*>(data) = type;
}

void MaterialNode::set_vec3(ParamType param_type, const Vec3& v)
{
    if (param_type == REFLECTANCE ||
        param_type == SPECULARITY ||
        param_type == RADIANCE    ||
        param_type == INT_IOR)
    {
        *((Vec3*)((uintptr_t)data + 4)) = v;
    }
    else if (param_type == TRANSMITTANCE ||
             param_type == EXT_IOR)
    {
        *((Vec3*)((uintptr_t)data + 8)) = v;
    }
}

void MaterialNode::set_float(ParamType param_type, float v)
{
    float* ptr = (float*)data;
    if (param_type == WEIGHT)
    {
        ptr[4] = v;
    }
    else if (param_type == INT_IOR)
    {
        ptr[12] = v;
    }
    else if (param_type == EXT_IOR)
    {
        ptr[13] = v;
    }
    else if (param_type == ROUGHNESS || param_type == SCALER)
    {
        ptr[14] = v;
    }
}

void MaterialNode::set_texture(ParamType param_type, int32_t texture)
{
    int32_t* ptr = (int32_t*)data;
    int32_t type = ptr[0];

    if (param_type == TRANSMITTANCE)
    {
        ptr[2] = texture;
    }
    else if (param_type == REFLECTANCE ||
             param_type == SPECULARITY ||
             param_type == RADIANCE    ||
             type == MIXMAP            ||
             type == BUMPMAP           ||
             type == NORMALMAP)
    {
        ptr[3] = texture;
    }
    else if (param_type == ROUGHNESS)
    {
        ptr[15] = texture;
    }
}

void MaterialNode::set_left_child(int32_t left)
{
    *((int32_t*)(data + 1)) = left;
}

void MaterialNode::set_right_child(int32_t right)
{
    *((int32_t*)(data + 2)) = right;
}

} // namespace eclipse
