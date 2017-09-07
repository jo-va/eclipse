#include "eclipse/scene/material_node.h"
#include "eclipse/scene/mat_expr.h"
#include "eclipse/math/vec3.h"

#include <string>
#include <cstring>
#include <cstdint>

namespace eclipse { namespace material {

template <typename T, typename F>
struct alias_cast_t
{
    union
    {
        F raw;
        T data;
    };
};

template <typename T, typename F>
T alias_cast(F raw_data)
{
    static_assert(sizeof(T) == sizeof(F), "Cannot cast types of different sizes");
    alias_cast_t<T, F> ac;
    ac.raw = raw_data;
    return ac.data;
}

Node::Node()
{
    memset(this, 0, sizeof(*this));
    int32_t* iptr = alias_cast<int32_t*>(data);

    // Set textures to -1
    iptr[1] = -1;
    iptr[2] = -1;
    iptr[3] = -1;
    iptr[15] = -1;
}

void Node::set_type(NodeType type)
{
    *alias_cast<NodeType*>(data) = type;
}

NodeType Node::get_type() const
{
    return *alias_cast<NodeType*>(data);
}

void Node::set_vec3(ParamType param_type, const Vec3& v)
{
    if (param_type == REFLECTANCE ||
        param_type == SPECULARITY ||
        param_type == RADIANCE    ||
        param_type == INT_IOR)
    {
        *alias_cast<Vec3*>(data + 4) = v;
    }
    else if (param_type == TRANSMITTANCE ||
             param_type == EXT_IOR)
    {
        *alias_cast<Vec3*>(data + 8) = v;
    }
}

void Node::set_float(ParamType param_type, float v)
{
    float* ptr = alias_cast<float*>(data);
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

void Node::set_texture(ParamType param_type, int32_t texture)
{
    int32_t* ptr = alias_cast<int32_t*>(data);
    int32_t type = ptr[0];

    if (param_type == TRANSMITTANCE)
    {
        ptr[2] = texture;
    }
    else if (param_type == REFLECTANCE ||
             param_type == SPECULARITY ||
             param_type == RADIANCE    ||
             type == OP_MIXMAP         ||
             type == OP_BUMPMAP        ||
             type == OP_NORMALMAP)
    {
        ptr[3] = texture;
    }
    else if (param_type == ROUGHNESS)
    {
        ptr[15] = texture;
    }
}

void Node::set_left_child(int32_t left)
{
    *alias_cast<int32_t*>(data + 1) = left;
}

void Node::set_right_child(int32_t right)
{
    *alias_cast<int32_t*>(data + 2) = right;
}

int32_t Node::get_left_child() const
{
    return *alias_cast<int32_t*>(data + 1);
}

int32_t Node::get_right_child() const
{
    return *alias_cast<int32_t*>(data + 2);
}

} } // namespace eclipse::material
