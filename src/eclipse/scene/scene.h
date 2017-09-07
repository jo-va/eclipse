#pragma once

#include "eclipse/scene/bvh_node.h"
#include "eclipse/scene/material_node.h"
#include "eclipse/scene/camera.h"
#include "eclipse/math/vec2.h"
#include "eclipse/math/vec4.h"
#include "eclipse/math/mat4.h"
#include "eclipse/util/texture.h"

#include <cstdint>
#include <vector>

namespace eclipse { namespace scene {

struct MeshInstance
{
    uint32_t mesh_index;
    uint32_t bvh_root;
    uint32_t padding[2];
    Mat4 transform;
};

struct TextureMetadata
{
    Texture::Format format;
    uint32_t width;
    uint32_t height;
    uint32_t offset;
};

struct EmissivePrimitive
{
    Mat4 transform;
    float area;
    uint32_t primitive_index;
    uint32_t material_index;
    uint32_t type;
};

enum EmissivePrimitiveType
{
    AreaLight,
    EnvironmentLight
};

struct Scene
{
    std::vector<bvh::Node> bvh_nodes;
    std::vector<MeshInstance> mesh_instances;
    std::vector<material::Node> material_nodes;
    std::vector<EmissivePrimitive> emissive_primitives;

    // Texture definitions and the associated data
    std::vector<uint8_t> texture_data;
    std::vector<TextureMetadata> texture_metadata;

    // Primitives are stored as an array of structs
    std::vector<Vec4> vertices;
    std::vector<Vec4> normals;
    std::vector<Vec2> uvs;
    std::vector<uint32_t> material_indices;

    // Indices to material nodes for storing the scene global
    // properties such as diffuse and emissive colors
    int32_t scene_diffuse_mat_index;
    int32_t scene_emissive_mat_index;

    Camera camera;
};

} } // namespace eclipse::scene
