#pragma once

#include "eclipse/math/bbox.h"
#include "eclipse/math/vec2.h"
#include "eclipse/math/vec3.h"
#include "eclipse/math/mat4.h"
#include "eclipse/math/bbox.h"

#include <vector>
#include <string>

namespace eclipse { namespace build {

struct Triangle
{
    Vec3 vertices[3];
    Vec3 normals[3];
    Vec2 uvs[3];
    uint32_t material_index;

    BBox bbox;
    Vec3 centroid;

    Triangle() : material_index(0) { }

    BBox get_bbox() const { return bbox; }
    Vec3 get_centroid() const { return centroid; }
};

struct Mesh
{
    std::string name;
    std::vector<Triangle*> triangles;

    BBox bbox;
    bool bbox_needs_update;

    Mesh(const std::string& name) : name(name), bbox_needs_update(true) { }

    void mark_bbox_dirty() { bbox_needs_update = true; }

    BBox get_bbox()
    {
        if (bbox_needs_update)
        {
            bbox = BBox();
            for (auto tri : triangles)
                bbox.merge(tri->get_bbox());
            bbox_needs_update = false;
        }
        return bbox;
    }
};

struct MeshInstance
{
    uint32_t mesh_index;
    Mat4 transform;

    BBox bbox;
    Vec3 centroid;

    BBox get_bbox() const { return bbox; }
    Vec3 get_centroid() const { return centroid; }
};

struct Material
{
    std::string name;
    std::string expression;
    std::string rel_path;
    bool used;

    Material() : used(false) { }
};

struct Camera
{
    float fov;
    Vec3 eye;
    Vec3 look_at;
    Vec3 up;

    Camera() : fov(45), eye(0, 0, 0), look_at(0, 0, -1), up(0, 1, 0) { }
};

struct InputScene
{
    std::vector<Mesh*> meshes;
    std::vector<MeshInstance*> mesh_instances;
    std::vector<Material*> materials;
    Camera camera;
};

} } // namespace eclipse::build
