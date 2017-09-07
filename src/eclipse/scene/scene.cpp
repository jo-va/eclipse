#include "eclipse/scene/scene.h"
#include "eclipse/util/except.h"

#include <istream>
#include <ostream>

namespace eclipse { namespace scene {

template <typename T>
void read_many(std::istream& is, T* value, size_t num)
{
    if (is.good())
        is.read((char*)value, sizeof(T) * num);
    if (is.fail())
        throw Error("read_many: failed to read scene from stream");
}

template <typename T>
void write_many(std::ostream& os, const T* value, size_t num)
{
    if (os.good())
        os.write((char*)value, sizeof(T) * num);
    if (os.fail())
        throw Error("write_many: failed to write scene to stream");
}

template <typename T>
void read_vec(std::istream& is, std::vector<T>& vec)
{
    size_t size;
    read_many(is, &size, 1);
    vec.resize(size);
    read_many(is, &vec[0], size);
}

template <typename T>
void write_vec(std::ostream& os, const std::vector<T>& vec)
{
    size_t size = vec.size();
    write_many(os, &size, 1);
    write_many(os, &vec[0], vec.size());
}

void Scene::deserialize(std::istream& is)
{
    read_vec(is, bvh_nodes);
    read_vec(is, mesh_instances);
    read_vec(is, material_nodes);
    read_vec(is, emissive_primitives);
    read_vec(is, texture_data);
    read_vec(is, texture_metadata);
    read_vec(is, vertices);
    read_vec(is, normals);
    read_vec(is, uvs);
    read_vec(is, material_indices);
    read_many(is, &scene_diffuse_mat_index, 1);
    read_many(is, &scene_emissive_mat_index, 1);
    read_many(is, &camera, 1);
}

void Scene::serialize(std::ostream& os)
{
    write_vec(os, bvh_nodes);
    write_vec(os, mesh_instances);
    write_vec(os, material_nodes);
    write_vec(os, emissive_primitives);
    write_vec(os, texture_data);
    write_vec(os, texture_metadata);
    write_vec(os, vertices);
    write_vec(os, normals);
    write_vec(os, uvs);
    write_vec(os, material_indices);
    write_many(os, &scene_diffuse_mat_index, 1);
    write_many(os, &scene_emissive_mat_index, 1);
    write_many(os, &camera, 1);
}

} } // namespace eclipse::scene

