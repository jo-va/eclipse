#pragma once

#include "eclipse/scene/bvh_node.h"
#include "eclipse/math/vec2.h"
#include "eclipse/math/vec4.h"
#include "eclipse/math/mat4.h"

#include <cstdint>
#include <vector>

namespace eclipse {

struct MeshInstance
{
    uint32_t mesh_index;
    uint32_t bvh_root;
    uint32_t padding[2];
    Mat4 transform;
};

struct Scene
{
    std::vector<bvh::Node> bvh_nodes;
    std::vector<MeshInstance> mesh_instances;

    // Primitives are stored as an array of structs
    std::vector<Vec4> vertices;
    std::vector<Vec4> normals;
    std::vector<Vec2> uvs;
    std::vector<uint32_t> material_indices;

    // Indices to material nodes for storing the scene global
    // properties such as diffuse and emissive colors
    int32_t scene_diffuse_mat_index;
    int32_t scene_emissive_mat_index;
};

} // namespace eclipse
