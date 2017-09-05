#include "eclipse/scene/compiler.h"
#include "eclipse/scene/scene.h"
#include "eclipse/scene/raw_scene.h"
#include "eclipse/scene/bvh_builder.h"
#include "eclipse/scene/mat_expr.h"
#include "eclipse/scene/known_ior.h"
#include "eclipse/util/logger.h"
#include "eclipse/util/stop_watch.h"
#include "eclipse/math/vec3.h"
#include "eclipse/math/vec4.h"
#include "eclipse/math/bbox.h"

#include <memory>
#include <map>
#include <cstring>

namespace eclipse { namespace scene {

namespace {

void create_layered_material_tree();
int32_t generate_material(raw::MaterialPtr material);
int32_t generate_material_tree(raw::MaterialPtr material, material::ExprNodePtr expr_node);
int32_t bake_texture(raw::MaterialPtr material, const std::string& texture);
void partition_geometry();
void setup_camera();

std::shared_ptr<raw::Scene> g_raw_scene;
std::unique_ptr<Scene> g_scene;

// A map of material indices to their layered material tree roots
std::map<uint32_t, int32_t> g_mat_index_to_mat_root;

// A list of material references for detecting circular loops
std::vector<std::string> g_mat_ref_list;

} // anonymous namespace

std::unique_ptr<Scene> compile(std::shared_ptr<raw::Scene> raw_scene)
{
    LOG_INFO("Compiling scene");
    StopWatch stop_watch;
    stop_watch.start();

    g_raw_scene = raw_scene;
    g_scene = std::make_unique<Scene>();
    g_scene->scene_diffuse_mat_index = -1;
    g_scene->scene_emissive_mat_index = -1;

    create_layered_material_tree();

    partition_geometry();

    setup_camera();

    stop_watch.stop();
    LOG_INFO("Compiled scene in ", stop_watch.get_elapsed_time_ms(), " ms");

    return std::move(g_scene);
}

namespace {

class MeshInstancePtrAccessor
{
public:
    MeshInstancePtrAccessor(const std::vector<raw::MeshInstancePtr>& items) { (void)items; }
    BBox get_bbox(const raw::MeshInstancePtr ptr) const { return ptr->get_bbox(); }
    Vec3 get_centroid(const raw::MeshInstancePtr ptr) const { return ptr->get_centroid(); }
};

class TriangleAccessor
{
public:
    TriangleAccessor(const std::vector<raw::Triangle>& items) { (void)items; }
    BBox get_bbox(const raw::Triangle tri) const { return tri.get_bbox(); }
    Vec3 get_centroid(const raw::Triangle tri) const { return tri.get_centroid(); }
};

// Generate a two-level BVH tree for the scene. The top level tree partitions
// the mesh instances. The bottom level trees are for the different meshes.
void partition_geometry()
{
    StopWatch stop_watch;
    stop_watch.start();
    LOG_INFO("Partitioning geometry");

    // Partition mesh instances so that each instance ends up in its own BVH leaf.
    LOG_INFO("Building scene BVH tree (", g_raw_scene->meshes.size(), " meshes, ",
             g_raw_scene->mesh_instances.size(), " mesh instances)");

    auto inst_leaf_cb = [&](bvh::Node* leaf, const std::vector<raw::MeshInstancePtr>& instances)
    {
        raw::MeshInstancePtr mi = instances[0];

        // Assign mesh instance index to node
        for (size_t i = 0; i < g_raw_scene->mesh_instances.size(); ++i)
        {
            if (g_raw_scene->mesh_instances[i] == mi)
            {
                leaf->set_mesh_index(uint32_t(i));
                break;
            }
        }
    };

    g_scene->bvh_nodes = bvh::Builder<raw::MeshInstancePtr, MeshInstancePtrAccessor,
         bvh::SAHStrategy<raw::MeshInstancePtr, MeshInstancePtrAccessor>>::build(
                 g_raw_scene->mesh_instances, 1, inst_leaf_cb);

    // Scan all meshes and calculate the size of material, vertex, normal
    // and uv lists; the pre-allocate them.
    size_t total_vertices = 0;
    for (auto& mesh : g_raw_scene->meshes)
        total_vertices += 3 * mesh->triangles.size();

    g_scene->vertices.reserve(total_vertices);
    g_scene->normals.reserve(total_vertices);
    g_scene->uvs.reserve(total_vertices);
    g_scene->material_indices.reserve(total_vertices / 3);

    // Partition each mesh into ist own BVH. Update all instances to point
    // to this mesh BVH.
    uint32_t vertex_offset = 0;
    uint32_t tri_offset = 0;
    std::vector<uint32_t> mesh_bvh_roots(g_raw_scene->meshes.size());

    for (size_t index = 0; index < g_raw_scene->meshes.size(); ++index)
    {
        auto& mesh = g_raw_scene->meshes[index];

        LOG_INFO("Building BVH tree for ", mesh->name, " (", mesh->triangles.size(), " triangles)");

        auto tri_leaf_cb = [&](bvh::Node* leaf, const std::vector<raw::Triangle>& triangles)
        {
            leaf->set_primitives(tri_offset, uint32_t(triangles.size()));

            // Copy triangles to flat arrays
            for (auto& tri : triangles)
            {
                g_scene->vertices[vertex_offset + 0] = Vec4(tri.vertices[0], 0.0f);
                g_scene->vertices[vertex_offset + 1] = Vec4(tri.vertices[1], 0.0f);
                g_scene->vertices[vertex_offset + 2] = Vec4(tri.vertices[2], 0.0f);

                g_scene->normals[vertex_offset + 0] = Vec4(tri.normals[0], 0.0f);
                g_scene->normals[vertex_offset + 1] = Vec4(tri.normals[1], 0.0f);
                g_scene->normals[vertex_offset + 2] = Vec4(tri.normals[2], 0.0f);

                g_scene->uvs[vertex_offset + 0] = tri.uvs[0];
                g_scene->uvs[vertex_offset + 1] = tri.uvs[1];
                g_scene->uvs[vertex_offset + 2] = tri.uvs[2];

                // TODO:Lookup root material node for primitive material index
                //uint32_t mat_node_index = g_mat_index_to_mat_root[tri.material_index];
                //g_scene->material_indices[tri_offset] = uint32_t(mat_node_index);

                // TODO:Find emissive materials here

                vertex_offset += 3;
                ++tri_offset;
            }
        };

        auto bvh_nodes = bvh::Builder<raw::Triangle, TriangleAccessor,
             bvh::SAHStrategy<raw::Triangle, TriangleAccessor>>::build(
                mesh->triangles, min_primitives_per_leaf, tri_leaf_cb);

        int32_t offset = (int32_t)g_scene->bvh_nodes.size();
        mesh_bvh_roots[index] = uint32_t(offset);
        for (size_t i = 0; i < bvh_nodes.size(); ++i)
            bvh_nodes[i].offset_child_nodes(offset);

        g_scene->bvh_nodes.insert(g_scene->bvh_nodes.end(), bvh_nodes.begin(), bvh_nodes.end());
    }

    // Process each mesh instance
    g_scene->mesh_instances.reserve(g_raw_scene->mesh_instances.size());
    for (size_t i = 0; i < g_raw_scene->mesh_instances.size(); ++i)
    {
        raw::MeshInstancePtr raw_mesh_inst = g_raw_scene->mesh_instances[i];
        MeshInstance* mesh_inst = &g_scene->mesh_instances[i];
        mesh_inst->mesh_index = raw_mesh_inst->mesh_index;
        mesh_inst->bvh_root = mesh_bvh_roots[raw_mesh_inst->mesh_index];

        // We need to invert the transformation matrix when performing ray traversal
        mesh_inst->transform = raw_mesh_inst->transform.inv;
    }

    stop_watch.stop();
    LOG_INFO("Partioned geometry in ", stop_watch.get_elapsed_time_ms(), " ms");
}

void setup_camera()
{
    // TODO:
}

// Parse material definitions into a node-based structure that models a layered material.
void create_layered_material_tree()
{
    StopWatch stop_watch;
    stop_watch.start();
    LOG_INFO("Processing ", g_raw_scene->materials.size(), " materials");

    for (size_t mat_index = 0; mat_index < g_raw_scene->materials.size(); ++mat_index)
    {
        raw::MaterialPtr mat = g_raw_scene->materials[mat_index];

        // Skip unused materials; those materials may be indirectly
        // reference from other used materials and will be lazilly
        // processed while compiling material expressions
        if (!mat->used)
            continue;

        LOG_INFO("Processing material ", mat->name);

        g_mat_index_to_mat_root[mat_index] = generate_material(mat);

        // TODO: fill emssive_index_cache

        if (mat->name == SceneDiffuseMaterialName)
            g_scene->scene_diffuse_mat_index = g_mat_index_to_mat_root[mat_index];
        if (mat->name == SceneEmissiveMaterialName)
            g_scene->scene_emissive_mat_index = g_mat_index_to_mat_root[mat_index];
    }

    stop_watch.stop();
    LOG_INFO("Processsed ", g_raw_scene->materials.size(), " materials in ", stop_watch.get_elapsed_time_ms(), " ms");
}

// Compile material expression and generate a layered material tree from it.
// This method returns back the root material tree node index.
int32_t generate_material(raw::MaterialPtr material)
{
    std::string error;
    material::ExprNodePtr expr_node = material::parse_expr(material->expression, error);
    if (!expr_node)
    {
        LOG_ERROR("Expression error for material '", material->name, "': ", error);
        return -1;
    }

    error = expr_node->validate();
    if (!error.empty())
    {
        LOG_ERROR("Expression error for material '", material->name, "': ", error);
        return -1;
    }

    g_mat_ref_list.push_back(material->name);

    // Create material node tree and store its root index
    return generate_material_tree(material, expr_node);
}

// Recursively construct an optimized material tree from the given expression.
// Return the index of the tree root in the scene's material node list.
int32_t generate_material_tree(raw::MaterialPtr material, material::ExprNodePtr expr_node)
{
    MaterialNode node;

    std::string error;
    float int_ior = material::get_known_ior(std::string(defaults::IntIOR), error);
    float ext_ior = material::get_known_ior(std::string(defaults::ExtIOR), error);

    node.set_float(MaterialNode::INT_IOR, int_ior);
    node.set_float(MaterialNode::EXT_IOR, ext_ior);

    if (auto mat_ref_node = std::dynamic_pointer_cast<material::NMatRef>(expr_node))
    {
        for (auto& name : g_mat_ref_list)
        {
            if (mat_ref_node->name == name)
            {
                std::string loop;
                for (size_t i = 0; i < g_mat_ref_list.size() - 1; ++i)
                    loop += g_mat_ref_list[i] + " => ";
                loop += g_mat_ref_list[g_mat_ref_list.size() - 1];
                LOG_ERROR("Detected circular dependency loop while processing ",
                          g_mat_ref_list[0], "; ", loop, mat_ref_node->name);
                return -1;
            }
        }

        for (auto& mat : g_raw_scene->materials)
        {
            if (mat->name == mat_ref_node->name)
                return generate_material(mat);
        }

        LOG_ERROR("Material ", material->name, " references undefined material ", mat_ref_node->name);
        return -1;
    }
    else if (auto bxdf_node = std::dynamic_pointer_cast<material::NBxdf>(expr_node))
    {

        if (bxdf_node->type == material::DIFFUSE)
        {
            node.set_type(MaterialNode::DIFFUSE);
            node.set_vec3(MaterialNode::REFLECTANCE, Vec3(defaults::Reflectance));
        }
        else if (bxdf_node->type == material::CONDUCTOR)
        {
            node.set_type(MaterialNode::CONDUCTOR);
            node.set_vec3(MaterialNode::SPECULARITY, Vec3(defaults::Specularity));
        }
        else if (bxdf_node->type == material::ROUGH_CONDUCTOR)
        {
            node.set_type(MaterialNode::ROUGH_CONDUCTOR);
            node.set_vec3(MaterialNode::SPECULARITY, Vec3(defaults::Specularity));
            node.set_float(MaterialNode::ROUGHNESS, defaults::Roughness);
        }
        else if (bxdf_node->type == material::DIELECTRIC)
        {
            node.set_type(MaterialNode::DIELECTRIC);
            node.set_vec3(MaterialNode::SPECULARITY, Vec3(defaults::Specularity));
            node.set_vec3(MaterialNode::TRANSMITTANCE, Vec3(defaults::Transmittance));
        }
        else if (bxdf_node->type == material::ROUGH_DIELECTRIC)
        {
            node.set_type(MaterialNode::ROUGH_DIELECTRIC);
            node.set_vec3(MaterialNode::SPECULARITY, Vec3(defaults::Specularity));
            node.set_vec3(MaterialNode::TRANSMITTANCE, Vec3(defaults::Transmittance));
            node.set_float(MaterialNode::ROUGHNESS, defaults::Roughness);
        }
        else if (bxdf_node->type == material::EMISSIVE)
        {
            node.set_type(MaterialNode::EMISSIVE);
            node.set_vec3(MaterialNode::RADIANCE, Vec3(defaults::Radiance));
            node.set_float(MaterialNode::SCALER, defaults::RadianceScaler);
        }
    }
    else if (auto mix_node = std::dynamic_pointer_cast<material::NMix>(expr_node))
    {

    }
    else if (auto mix_map_node = std::dynamic_pointer_cast<material::NMixMap>(expr_node))
    {

    }
    else if (auto bump_map_node = std::dynamic_pointer_cast<material::NBumpMap>(expr_node))
    {

    }
    else if (auto normal_map_node = std::dynamic_pointer_cast<material::NNormalMap>(expr_node))
    {

    }
    else if (auto disperse_node = std::dynamic_pointer_cast<material::NDisperse>(expr_node))
    {

    }

    g_scene->material_nodes.push_back(node);

    return int32_t(g_scene->material_nodes.size() - 1);
}

int32_t bake_texture(raw::MaterialPtr material, const std::string& texture)
{
    return -1;
}

} // anonymous namespace

} } // namespace eclipse::scene
