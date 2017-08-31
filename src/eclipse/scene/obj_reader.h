#pragma once

#include "eclipse/scene/compiler/input_scene_types.h"
#include "eclipse/math/vec2.h"
#include "eclipse/math/vec3.h"

#include <cstddef>
#include <string>
#include <map>
#include <vector>

namespace eclipse {

class Resource;

class Scene
{

};

struct OBJMaterial
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
    Resource* resource;
    bool used;

    std::string get_expression();
};

class SceneReader
{
public:
    virtual Scene* read(Resource* scene) = 0;
};

class OBJReader : public SceneReader
{
public:
    OBJReader();
    ~OBJReader();

    Scene* read(Resource* scene) override;

private:
    void parse(Resource* res);
    void parse_materials(Resource* res);

    void create_default_mesh_instances();
    void process_materials();
    OBJMaterial* default_material();

    std::vector<build::Triangle*> parse_face(const std::vector<std::string>& tokens, size_t rel_vertex_offset,
                                             size_t rel_normal_offset, size_t rel_uv_offset);

    build::MeshInstance* parse_mesh_instance(const std::vector<std::string>& tokens);

    size_t select_coord_index(const std::string& index_token, size_t coord_list_size, size_t rel_offset);

    void verify_last_parsed_mesh();

    float parse_float(const std::vector<std::string>& tokens);
    Vec2 parse_vec2(const std::vector<std::string>& tokens);
    Vec3 parse_vec3(const std::vector<std::string>& tokens);

    void push_call(const std::string& msg);
    void pop_call();
    std::string get_call_stack();

private:
    build::InputScene* m_input_scene;
    std::map<std::string, size_t> m_mat_name_to_index_map;
    OBJMaterial* m_current_mat;
    std::vector<OBJMaterial*> m_materials;

    std::vector<Vec3> m_vertices;
    std::vector<Vec3> m_normals;
    std::vector<Vec2> m_uvs;

    std::vector<std::string> m_call_stack;
    const char* m_file;
    size_t m_line_num;
};

} // namespace eclipse
