#include "eclipse/scene/obj_loader.h"
#include "eclipse/scene/raw_scene.h"
#include "eclipse/scene/compiler.h"
#include "eclipse/scene/mat_expr.h"
#include "eclipse/math/vec2.h"
#include "eclipse/math/vec3.h"
#include "eclipse/math/transform.h"
#include "eclipse/util/logger.h"
#include "eclipse/util/stop_watch.h"
#include "eclipse/util/resource.h"

#include <cstdint>
#include <cstddef>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <sstream>
#include <memory>

namespace eclipse { namespace scene {

namespace {

struct Material
{
    std::string name;
    Vec3 Kd;
    Vec3 Ks;
    Vec3 Ke;
    float Ke_scaler;
    Vec3 Tf;
    float Ni;
    std::string Kd_tex;
    std::string Ks_tex;
    std::string Ke_tex;
    std::string Tf_tex;
    std::string bump_tex;
    std::string normal_tex;
    std::string expression;
    std::shared_ptr<Resource> resource;
    bool used;

    std::string get_expression();

    ~Material()
    {
        //LOG_DEBUG("obj::Material ", name, " deleted");
    }
};

void parse(std::shared_ptr<Resource> res);

std::vector<raw::Triangle> parse_face(const std::vector<std::string>& tokens, size_t vert_off, size_t norm_off, size_t uv_off);
std::shared_ptr<raw::MeshInstance> parse_mesh_instance(const std::vector<std::string>& tokens);
void create_default_mesh_instances();
size_t select_coord_index(const std::string& index_token, size_t coord_list_size, size_t rel_offset);
void verify_last_parsed_mesh();

void parse_materials(std::shared_ptr<Resource> res);
void process_materials();
Material* default_material();

float parse_float(const std::vector<std::string>& tokens);
Vec2 parse_vec2(const std::vector<std::string>& tokens);
Vec3 parse_vec3(const std::vector<std::string>& tokens);

void push_call(const std::string& msg);
void pop_call();
std::string get_call_stack();

std::unique_ptr<raw::Scene> g_raw_scene;

std::map<std::string, size_t> g_mat_name_to_index_map;
std::vector<Material*> g_materials;
Material* g_current_mat;

std::vector<Vec3> g_vertices;
std::vector<Vec3> g_normals;
std::vector<Vec2> g_uvs;

std::vector<std::string> g_call_stack;
const char* g_file;
size_t g_line_num;

size_t g_num_triangles;
size_t g_num_vertices;

} // anonymous namespace

std::unique_ptr<raw::Scene> load_obj(std::shared_ptr<Resource> scene)
{
    g_mat_name_to_index_map.clear();
    g_materials.clear();
    g_vertices.clear();
    g_normals.clear();
    g_uvs.clear();
    g_call_stack.clear();
    g_current_mat = nullptr;
    g_file = nullptr;
    g_line_num = 0;
    g_num_triangles = 0;
    g_num_vertices = 0;
    g_raw_scene = std::make_unique<raw::Scene>();

    LOG_INFO("Parsing scene from ", scene->get_path());

    StopWatch stopwatch;
    stopwatch.start();

    parse(scene);

    if (g_raw_scene->mesh_instances.empty())
        create_default_mesh_instances();

    process_materials();

    for (size_t i = 0; i < g_materials.size(); ++i)
        delete g_materials[i];

    stopwatch.stop();
    LOG_INFO("Parsed scene in ", stopwatch.get_elapsed_time_ms(), " ms [",
             g_raw_scene->mesh_instances.size(), " mesh instances - ",
             g_num_vertices, " vertices - ", g_num_triangles, " triangles]");

    return std::move(g_raw_scene);
}

namespace {

std::string Material::get_expression()
{
    if (!expression.empty())
        return expression;

    bool is_specular_reflection = max_component(Ks) > 0.0f || !Ks_tex.empty();
    bool is_emissive = max_component(Ke) > 0.0f || !Ke_tex.empty();

    material::NodeType bxdf;
    std::vector<std::string> expr_args;

    if (is_specular_reflection && Ni == 0.0f)
    {
        bxdf = material::BXDF_CONDUCTOR;
        if (!Ks_tex.empty())
            expr_args.push_back(std::string(material::Specularity) + ": " + Ks_tex);
        else if (max_component(Ks) > 0.0f)
            expr_args.push_back(std::string(material::Specularity) + ": " + to_string(Ks));
    }
    else if (is_specular_reflection && Ni != 0.0f)
    {
        bxdf = material::BXDF_DIELECTRIC;
        if (!Ks_tex.empty())
            expr_args.push_back(std::string(material::Specularity) + ": " + Ks_tex);
        else if (max_component(Ks) > 0.0f)
            expr_args.push_back(std::string(material::Specularity) + ": " + to_string(Ks));

        if (!Tf_tex.empty())
            expr_args.push_back(std::string(material::Transmittance) + ": " + Tf_tex);
        else if (max_component(Tf) > 0.0f)
            expr_args.push_back(std::string(material::Transmittance) + ": " + to_string(Tf));

        expr_args.push_back(std::string(material::IntIOR) + ": " + std::to_string(Ni));
    }
    else if (is_emissive)
    {
        bxdf = material::BXDF_EMISSIVE;
        if (!Ke_tex.empty())
            expr_args.push_back(std::string(material::Radiance) + ": " + Ke_tex);
        else if (max_component(Ke) > 0.0f)
            expr_args.push_back(std::string(material::Radiance) + ": " + to_string(Ke));

        if (Ke_scaler != 0.0f)
            expr_args.push_back(std::string(material::Scale) + ": " + std::to_string(Ke_scaler));
    }
    else
    {
        bxdf = material::BXDF_DIFFUSE;
        if (!Kd_tex.empty())
            expr_args.push_back(std::string(material::Reflectance) + ": " + Kd_tex);
        else if (max_component(Kd) > 0.0f)
            expr_args.push_back(std::string(material::Reflectance) + ": " + to_string(Kd));
    }

    std::string joined_expr_args;
    for (auto const& s : expr_args)
        joined_expr_args += ", " + s;

    std::string expr = material::node_to_string(bxdf) + "(" + joined_expr_args + ")";

    if (!normal_tex.empty())
        expr = "normalMap(" + expr + ", " + normal_tex + ")";
    else if (!bump_tex.empty())
        expr = "bumpMap(" + expr + ", " + bump_tex + ")";

    return expr;
}

void push_call(const std::string& msg)
{
    g_call_stack.push_back(msg);
}

void pop_call()
{
    g_call_stack.pop_back();
}

std::string get_call_stack()
{
    std::string msg = "\n";
    for (auto it = g_call_stack.rbegin(); it != g_call_stack.rend(); ++it)
        msg += *it + "\n";
    return msg;
}

// Generate a mesh instance with an identity transformation for each defined mesh
void create_default_mesh_instances()
{
    for (size_t i = 0; i < g_raw_scene->meshes.size(); ++i)
    {
        auto mesh = g_raw_scene->meshes[i];

        auto inst = std::make_shared<raw::MeshInstance>();
        inst->mesh_index = uint32_t(i);
        inst->transform = Transform();
        inst->bbox = mesh->get_bbox();
        inst->centroid = mesh->get_bbox().get_centroid();

        g_raw_scene->mesh_instances.push_back(inst);
    }
}

// Generate scene materials for material entries that are in use and update
// the material indices for all parsed primitives
void process_materials()
{
    std::map<int, int> obj_mat_to_scene_mat;
    std::vector<std::shared_ptr<raw::Material>> pruned_materials;
    size_t pruned = 0;

    for (size_t obj_mat_idx = 0; obj_mat_idx < g_materials.size(); ++obj_mat_idx)
    {
        Material* obj_mat = g_materials[obj_mat_idx];

        // whitelist scene materials
        if (obj_mat->name == std::string(SceneDiffuseMaterialName) ||
            obj_mat->name == std::string(SceneEmissiveMaterialName))
            obj_mat->used = true;

        // Prune unused materials
        if (!obj_mat->used)
        {
            LOG_WARNING("Skipping unused material ", obj_mat->name);
            auto pruned_mat = std::make_shared<raw::Material>();
            pruned_mat->name = obj_mat->name;
            pruned_mat->expression = obj_mat->get_expression();
            pruned_mat->resource = obj_mat->resource;
            pruned_materials.push_back(pruned_mat);

            ++pruned;
            continue;
        }

        auto mat = std::make_shared<raw::Material>();
        mat->name = obj_mat->name;
        mat->expression = obj_mat->get_expression();
        mat->resource = obj_mat->resource;
        mat->used = true;
        g_raw_scene->materials.push_back(mat);

        obj_mat_to_scene_mat[obj_mat_idx] = g_raw_scene->materials.size() - 1;
    }

    // For each primitive, map obj material indices to the generated materials
    for (auto& mesh : g_raw_scene->meshes)
        for (auto& tri : mesh->triangles)
            tri.material_index = obj_mat_to_scene_mat[tri.material_index];

    // Append pruned materials at the end of the list as they may be
    // reference by material expressions
    g_raw_scene->materials.insert(g_raw_scene->materials.end(),
                                  pruned_materials.begin(),
                                  pruned_materials.end());

    if (pruned > 0)
        LOG_INFO("Pruned ", pruned, " unused materials");
}

// Create and select a default material for surfaces not using one
Material* default_material()
{
    std::string mat_name = "";

    size_t mat_index;
    auto it = g_mat_name_to_index_map.find(mat_name);

    if (it == g_mat_name_to_index_map.end())
    {
        Material* mat = new Material();
        mat->Kd = Vec3(0.7, 0.7, 0.7);
        g_materials.push_back(mat);

        mat_index = g_materials.size() - 1;
        g_mat_name_to_index_map[mat_name] = mat_index;
    }
    else
    {
        mat_index = it->second;
    }

    g_current_mat = g_materials[mat_index];

    return g_current_mat;
}

void parse(std::shared_ptr<Resource> res)
{
    size_t rel_vertex_offset = g_vertices.size();
    size_t rel_normal_offset = g_normals.size();
    size_t rel_uv_offset = g_uvs.size();

    std::istream& input_stream = res->get_stream();
    std::vector<std::string> tokens(100);

    size_t line_num = 0;
    for (std::string line; std::getline(input_stream, line);)
    {
        ++line_num;
        g_line_num = line_num;
        g_file = res->get_path().c_str();
        //LOG_DEBUG("[", line_num, "]: ", line);

        // Tokenize line
        std::stringstream line_stream(line);
        tokens.clear();
        std::string token;
        while (line_stream >> token)
            tokens.push_back(token);

        if (tokens.empty() || tokens[0][0] == '#')
            continue;

        if (tokens[0] == "call" || tokens[0] == "mtllib")
        {
            if (tokens.size() != 2)
                LOG_ERROR(res->get_path(), ": ", line_num,
                        " -> unsupported syntax for ", tokens[0],
                        "; expected 1 argument; got ", tokens.size() - 1, get_call_stack());

            push_call("Referenced from " + tokens[1] + ": " + std::to_string(line_num) + " [" + tokens[0] + "]");

            auto inner_res = std::make_shared<Resource>(tokens[1], res);
            if (tokens[0] == "call")
                parse(inner_res);
            else
                parse_materials(inner_res);

            pop_call();
        }
        else if (tokens[0] == "usemtl")
        {
            if (tokens.size() != 2)
                LOG_ERROR(res->get_path(), ": ", line_num,
                        " -> unsupported syntax for \"usemtl\" \
                        ; expected 1 argument; got ", tokens.size() - 1, get_call_stack());

            if (g_mat_name_to_index_map.find(tokens[1]) == g_mat_name_to_index_map.end())
                LOG_ERROR(res->get_path(), ": ", line_num,
                        " -> undefined material with name ", tokens[1], get_call_stack());

            size_t mat_index = g_mat_name_to_index_map[tokens[1]];
            g_current_mat = g_materials[mat_index];
        }
        else if (tokens[0] == "v")
        {
            g_vertices.push_back(parse_vec3(tokens));
            ++g_num_vertices;
        }
        else if (tokens[0] == "vn")
        {
            g_normals.push_back(parse_vec3(tokens));
        }
        else if (tokens[0] == "vt")
        {
            g_uvs.push_back(parse_vec2(tokens));
        }
        else if (tokens[0] == "g" || tokens[0] == "o")
        {
            if (tokens.size() < 2)
                LOG_ERROR(res->get_path(), ": ", line_num, " -> unsupported syntax for ", tokens[0],
                          "; expected 1 argument; got ", tokens.size() - 1);

            verify_last_parsed_mesh();
            g_raw_scene->meshes.push_back(std::make_shared<raw::Mesh>(tokens[1]));
        }
        else if (tokens[0] == "f")
        {
            std::vector<raw::Triangle> triangles = parse_face(tokens, rel_vertex_offset, rel_normal_offset, rel_uv_offset);
            g_num_triangles += triangles.size();

            if (g_raw_scene->meshes.size() == 0)
                g_raw_scene->meshes.push_back(std::make_shared<raw::Mesh>("default"));

            size_t mesh_index = g_raw_scene->meshes.size() - 1;
            auto mesh = g_raw_scene->meshes[mesh_index];
            mesh->mark_bbox_dirty();
            mesh->triangles.insert(mesh->triangles.end(), triangles.begin(), triangles.end());
        }
        else if (tokens[0] == "camera_fov")
        {
            g_raw_scene->camera.fov = parse_float(tokens);
        }
        else if (tokens[0] == "camera_eye")
        {
            g_raw_scene->camera.eye = parse_vec3(tokens);
        }
        else if (tokens[0] == "camera_look")
        {
            g_raw_scene->camera.look_at = parse_vec3(tokens);
        }
        else if (tokens[0] == "camera_up")
        {
            g_raw_scene->camera.up = parse_vec3(tokens);
        }
        else if (tokens[0] == "instance")
        {
            auto instance = parse_mesh_instance(tokens);
            g_raw_scene->mesh_instances.push_back(instance);
        }
    }

    verify_last_parsed_mesh();
}

void verify_last_parsed_mesh()
{
    int last_mesh_index = g_raw_scene->meshes.size() - 1;
    if (last_mesh_index >= 0 && g_raw_scene->meshes[last_mesh_index]->triangles.size() == 0)
    {
        LOG_WARNING("Dropping mesh ", g_raw_scene->meshes[last_mesh_index]->name, " as it contains no polygons");

        //delete g_raw_scene->meshes[last_mesh_index];
        g_raw_scene->meshes.pop_back();
    }
}

void parse_materials(std::shared_ptr<Resource> res)
{
    LOG_INFO("Parsing material library ", res->get_path());

    std::istream& input_stream = res->get_stream();
    std::vector<std::string> tokens(100);

    Material* current_mat = nullptr;
    std::string mat_name;

    size_t line_num = 0;
    for (std::string line; std::getline(input_stream, line);)
    {
        ++line_num;
        g_line_num = line_num;
        g_file = res->get_path().c_str();

        //LOG_DEBUG("[", line_num, "]: ", line);

        // Tokenize line
        std::stringstream line_stream(line);
        tokens.clear();
        std::string token;
        while (line_stream >> token)
            tokens.push_back(token);

        if (tokens.empty() || tokens[0][0] == '#')
            continue;

        if (tokens[0] == "newmtl")
        {
            if (tokens.size() != 2)
                LOG_ERROR(res->get_path(), ": ", line_num,
                          " -> unsupported syntax for \"newmtl\"; \
                          expected 1 argument; got ", tokens.size() - 1, get_call_stack());

            mat_name = tokens[1];
            if (g_mat_name_to_index_map.find(mat_name) != g_mat_name_to_index_map.end())
                LOG_ERROR(res->get_path(), ": ", line_num,
                          " -> material ", mat_name, " already defined", get_call_stack());

            Material* material = new Material();
            material->name = mat_name;
            material->resource = res;
            g_materials.push_back(material);

            current_mat = g_materials[g_materials.size() - 1];
            g_mat_name_to_index_map[mat_name] = g_materials.size() - 1;
        }
        else
        {
            if (current_mat == nullptr)
                LOG_ERROR(res->get_path(), ": ", line_num, " -> got ",
                          tokens[0], " withoug a \"newmtl\"", get_call_stack());

            if (tokens[0] == "include")
            {
                if (tokens.size() < 2)
                    LOG_ERROR(res->get_path(), ": ", line_num,
                             " -> unsupported syntax for \"include\"; expected 1 argument; \
                             got ", tokens.size() - 1, get_call_stack());

                auto it = g_mat_name_to_index_map.find(tokens[1]);
                if (it == g_mat_name_to_index_map.end())
                    LOG_ERROR();

                *current_mat = *g_materials[it->second];
                current_mat->name = mat_name;
            }
            else if (tokens[0] == "Kd")
            {
                current_mat->Kd = parse_vec3(tokens);
            }
            else if (tokens[0] == "Ks")
            {
                current_mat->Ks = parse_vec3(tokens);
            }
            else if (tokens[0] == "Ke")
            {
                current_mat->Ke = parse_vec3(tokens);
            }
            else if (tokens[0] == "Tf")
            {
                current_mat->Tf = parse_vec3(tokens);
            }
            else if (tokens[0] == "Ni")
            {
                current_mat->Ni = parse_float(tokens);
            }
            else if (tokens[0] == "map_Kd")
            {
                current_mat->Kd_tex = tokens[1];
            }
            else if (tokens[0] == "map_Ks")
            {
                current_mat->Ks_tex = tokens[1];
            }
            else if (tokens[0] == "map_Ke")
            {
                current_mat->Ke_tex = tokens[1];
            }
            else if (tokens[0] == "map_Tf")
            {
                current_mat->Tf_tex = tokens[1];
            }
            else if (tokens[0] == "map_bump")
            {
                current_mat->bump_tex = tokens[1];
            }
            else if (tokens[0] == "map_normal")
            {
                current_mat->normal_tex = tokens[1];
            }
            else if (tokens[0] == "mat_expr")
            {
                if (tokens.size() < 2)
                    LOG_ERROR(res->get_path(), ": ", line_num, " -> unsupported syntax for \
                            \"mat_expr\"; expected 1 argument; got ", tokens.size() - 1, get_call_stack());

                for (size_t i = 1; i < tokens.size() - 1; ++i)
                    current_mat->expression += tokens[i] + " ";
                current_mat->expression += tokens[tokens.size() - 1];
            }
            else if (tokens[0] == "KeScaler")
            {
                if (tokens.size() < 2)
                    LOG_ERROR(res->get_path(), ": ", line_num, " -> unsupported syntax for \
                              \"KeScaler\"; expected 1 argument; got ", tokens.size() - 1, get_call_stack());

                current_mat->Ke_scaler = parse_float(tokens);
            }
        }
    }
}

std::vector<raw::Triangle> parse_face(const std::vector<std::string>& tokens,
                                      size_t rel_vertex_offset,
                                      size_t rel_normal_offset,
                                      size_t rel_uv_offset)
{
    std::vector<raw::Triangle> triangles;
    if (tokens.size() < 4 || tokens.size() > 5)
    {
        LOG_ERROR(g_file, ": ", g_line_num, " -> unsupported syntax for \"f\"; expected 3 arguments \
                  for triangular faces or 4 arguments for a quad face; got ", tokens.size() - 1,
                  "Select the triangulation option in your exporter");
        return triangles;
    }

    Vec3 vertices[4];
    Vec3 normals[4];
    Vec2 uvs[4];
    bool has_normals = false;
    size_t exp_indices = 0;

    for (size_t arg = 0; arg < tokens.size() - 1; ++arg)
    {
        // Tokenize the line arg1/arg2/arg3
        //                   arg1//arg3
        //                   arg1/arg2
        //                   arg1
        std::vector<std::string> face_tokens;
        const char* c = tokens[arg + 1].c_str();
        std::string token;
        while (1)
        {
            if (*c == '/' || *c == '\0')
            {
                face_tokens.push_back(token);
                token.clear();
                if (*c == '\0')
                    break;
            }
            else
            {
                token += *c;
            }
            ++c;
        }

        // The first triangle defines the format for the following args
        if (arg == 0)
        {
            exp_indices = face_tokens.size();
        }
        else if (face_tokens.size() != exp_indices)
        {
            LOG_ERROR(g_file, ": ", g_line_num, " -> expected each face argument to contain ",
                      exp_indices, "indices; arg ", arg, " contains ", face_tokens.size(), " indices");
            return triangles;
        }

        // Faces must at least define a vertex coord
        if (face_tokens[0].empty())
        {
            LOG_ERROR(g_file, ": ", g_line_num, " -> face argument ", arg, " does not include a vertex index");
            return triangles;
        }

        int offset = select_coord_index(face_tokens[0], g_vertices.size(), rel_vertex_offset);
        vertices[arg] = g_vertices[offset];

        // Parse uv coords if specified
        if (exp_indices > 1 && !face_tokens[1].empty())
        {
            offset = select_coord_index(face_tokens[1], g_uvs.size(), rel_uv_offset);
            uvs[arg] = g_uvs[offset];
        }

        // Parse normal coords if specified
        if (exp_indices > 2 && !face_tokens[2].empty())
        {
            offset = select_coord_index(face_tokens[2], g_normals.size(), rel_normal_offset);
            normals[arg] = g_normals[offset];
            has_normals = true;
        }
    }

    // If no material defined select the default. Also flag the current material
    // as being in use so we don't prune it later
    if (g_current_mat == nullptr)
        g_current_mat = default_material();
    g_current_mat->used = true;

    // If no normals are available generate them from the vertices
    if (!has_normals)
    {
        const Vec3 e01 = vertices[1] - vertices[0];
        const Vec3 e02 = vertices[2] - vertices[0];
        const Vec3 normal = normalize(cross(e01, e02));
        normals[0] = normal;
        normals[1] = normal;
        normals[2] = normal;
        normals[3] = normal;
    }

    // Assemble vertices into one or two primitives depending on whether we are parsing
    // a triangular or a quad face
    size_t indices_list[2][3] = { { 0, 1, 2 }, { 0, 2, 3 } };
    size_t num_tris = (tokens.size() == 4) ? 1 : 2;

    for (size_t i = 0; i < num_tris; ++i)
    {
        size_t* indices = &indices_list[i][0];

        raw::Triangle tri;
        tri.material_index = g_mat_name_to_index_map[g_current_mat->name];

        for (size_t j = 0; j < 3; ++j)
        {
            size_t index = indices[j];
            tri.vertices[j] = vertices[index];
            tri.normals[j] = normals[index];
            tri.uvs[j] = uvs[index];
        }

        tri.bbox = BBox(tri.vertices[0], tri.vertices[1], tri.vertices[2]);
        tri.centroid = (tri.vertices[0] + tri.vertices[1] + tri.vertices[2]) * (1.0f / 3.0f);

        triangles.push_back(std::move(tri));
    }

    return triangles;
}

size_t select_coord_index(const std::string& index_token, size_t coord_list_size, size_t rel_offset)
{
    int index = std::stoi(index_token);

    int offset = 0;
    if (index < 0)
        offset = int(coord_list_size) + index;
    else
        offset = int(rel_offset) + index - 1;

    if (offset < 0 || size_t(offset) >= coord_list_size)
        LOG_ERROR(g_file, ": ", g_line_num, " -> index out of bounds");

    return offset;
}

std::shared_ptr<raw::MeshInstance> parse_mesh_instance(const std::vector<std::string>& tokens)
{
    if (tokens.size() != 11)
    {
        LOG_ERROR(g_file, ": ", g_line_num, " -> unsupported syntax for \"instance\";\
                  expected 10 arguments: mesh_name tX tY tZ yaw pitch roll scaleX scaleY scaleZ; got ", tokens.size() - 1);
        return nullptr;;
    }

    // Find object by name
    std::string mesh_name = tokens[1];
    int mesh_index = -1;
    for (size_t i = 0; i < g_raw_scene->meshes.size(); ++i)
    {
        auto mesh = g_raw_scene->meshes[i];
        if (mesh->name == mesh_name)
        {
            mesh_index = int(i);
            break;
        }
    }
    if (mesh_index == -1)
    {
        LOG_ERROR(g_file, ": ", g_line_num, " -> unknown mesh with name ", mesh_name);
        return nullptr;
    }

    Vec3 translation;
    Vec3 rotation;
    Vec3 scaling;

    // Parse translation
    for (size_t i = 2; i < 5; ++i)
        translation[i - 2] = std::stof(tokens[i]);

    // Parse rotation angles and convert to radians
    for (size_t i = 5; i < 8; ++i)
        rotation[i - 5] = std::stof(tokens[i]) * (float)pi / 180.0f;

    // Parse scale
    for (size_t i = 8; i < 11; ++i)
        scaling[i - 8] = std::stof(tokens[i]);

    // Generate final matrix: M = T * R * S
    Transform scale_xfm = scale(scaling);
    Transform trans_xfm = translate(translation);
    Transform rot_xfm = rotate_z(rotation[2]) * rotate_y(rotation[1]) * rotate_x(rotation[0]);
    Transform total_xfm = trans_xfm * rot_xfm * scale_xfm;

    // Get mesh bbox and recalculate a new BBox for the mesh instance
    BBox mesh_bbox = g_raw_scene->meshes[mesh_index]->get_bbox();
    BBox inst_bbox = transform_bbox(total_xfm, mesh_bbox);

    auto instance = std::make_shared<raw::MeshInstance>();
    instance->mesh_index = uint32_t(mesh_index);
    instance->bbox = inst_bbox;
    instance->centroid = inst_bbox.get_centroid();
    instance->transform = total_xfm;

    return instance;
}

float parse_float(const std::vector<std::string>& tokens)
{
    if (tokens.size() < 2)
    {
        LOG_ERROR(g_file, ": ", g_line_num,
                  " -> unsupported syntax for ", tokens[0],
                  "; expected 1 arguments; got ", tokens.size() - 1, get_call_stack());
        return 0.0f;
    }

    return std::stof(tokens[1]);
}

Vec2 parse_vec2(const std::vector<std::string>& tokens)
{
    if (tokens.size() < 3)
    {
        LOG_ERROR(g_file, ": ", g_line_num,
                  " -> Unsupported syntax for ", tokens[0],
                  "; expected 2 arguments; got ", tokens.size() - 1, get_call_stack());
        return Vec2();
    }

    return Vec2(std::stof(tokens[1]), std::stof(tokens[2]));
}

Vec3 parse_vec3(const std::vector<std::string>& tokens)
{
    if (tokens.size() < 4)
    {
        LOG_ERROR(g_file, ": ", g_line_num,
                  " -> Unsupported syntax for ", tokens[0],
                  "; expected 3 arguments; got ", tokens.size() - 1, get_call_stack());
        return Vec3();
    }

    return Vec3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
}

} // anonymous namespace

} } // namespace eclipse::scene
