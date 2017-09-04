#pragma once

#include "eclipse/math/bbox.h"
#include "eclipse/math/vec2.h"
#include "eclipse/math/vec3.h"
#include "eclipse/math/bbox.h"
#include "eclipse/math/transform.h"
#include "eclipse/scene/resource.h"
#include "eclipse/util/logger.h"

#include <vector>
#include <string>
#include <memory>

namespace eclipse { namespace raw {

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
    std::vector<Triangle> triangles;

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
                bbox.merge(tri.get_bbox());
            bbox_needs_update = false;
        }
        return bbox;
    }

    ~Mesh()
    {
        LOG_DEBUG("Mesh ", name, " deleted");
    }
};

typedef std::shared_ptr<Mesh> MeshPtr;

struct MeshInstance
{
    uint32_t mesh_index;
    Transform transform;

    BBox bbox;
    Vec3 centroid;

    BBox get_bbox() const { return bbox; }
    Vec3 get_centroid() const { return centroid; }

    ~MeshInstance()
    {
        LOG_DEBUG("MeshInstance ", mesh_index, " deleted");
    }
};

typedef std::shared_ptr<MeshInstance> MeshInstancePtr;

struct Material
{
    std::string name;
    std::string expression;
    std::shared_ptr<Resource> resource;
    bool used;

    Material() : used(false) { }

    ~Material()
    {
        LOG_DEBUG("Material ", name, " deleted");
    }
};

typedef std::shared_ptr<Material> MaterialPtr;

struct Camera
{
    float fov;
    Vec3 eye;
    Vec3 look_at;
    Vec3 up;

    Camera() : fov(45), eye(0, 0, 0), look_at(0, 0, -1), up(0, 1, 0) { }
};

struct Scene
{
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<MeshInstance>> mesh_instances;
    std::vector<std::shared_ptr<Material>> materials;
    Camera camera;
};

} } // namespace eclipse::raw
