#include "eclipse/scene/scene.h"
#include "eclipse/util/except.h"

#include <string>
#include <sstream>
#include <istream>
#include <ostream>
#include <iomanip>
#include <cstdio>

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

void Scene::serialize(std::ostream& os) const
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

std::string size_str(float size)
{
    char buff[100] = { 0 };
    if (size < 1024)
        std::snprintf(buff, sizeof(buff), "%3d bytes", (int)size);
    else if (size < 1024 * 1024)
        std::snprintf(buff, sizeof(buff), "%3.1f kb", size / 1024);
    else
        std::snprintf(buff, sizeof(buff), "%5.1f mb", size / (1024 * 1024));
    return std::string(buff);
}

template <typename T>
size_t vec_size(const std::vector<T>& v)
{
    return sizeof(T) * v.size();
}

template <typename T>
std::string vec_size_str(const std::vector<T>& v)
{
    return size_str(vec_size(v));
}

std::string Scene::get_stats() const
{
    std::stringstream ss;

    ss << "scene statistics:\n\n";

    size_t total_size = vec_size(vertices) + vec_size(normals) + vec_size(uvs) + vec_size(bvh_nodes) +
                        vec_size(mesh_instances) + vec_size(emissive_primitives) +
                        vec_size(material_indices) + vec_size(material_nodes) +
                        vec_size(texture_metadata) + vec_size(texture_data);
    size_t col1w = 18;
    size_t col2w = 18;
    size_t col3w = 18;
    size_t totalw = col1w + col2w + col3w;
    size_t titleoff = totalw / 2;

    ss << std::setw(titleoff - 4) << ' ' << "Geometry" << "\n"
       << " " << std::setfill('-') << std::setw(totalw) << '-' << "\n" << std::setfill(' ');

    ss << std::setw(col1w) << "Vertices: "  << std::setw(col2w) << vertices.size()  << std::setw(col3w) << vec_size_str(vertices)  << "\n"
       << std::setw(col1w) << "Normals: "   << std::setw(col2w) << normals.size()   << std::setw(col3w) << vec_size_str(normals)   << "\n"
       << std::setw(col1w) << "UVs: "       << std::setw(col2w) << uvs.size()       << std::setw(col3w) << vec_size_str(uvs)       << "\n"
       << std::setw(col1w) << "BVH nodes: " << std::setw(col2w) << bvh_nodes.size() << std::setw(col3w) << vec_size_str(bvh_nodes) << "\n\n";

    ss << std::setw(titleoff - 7) << ' ' << "Mesh/emissives" << "\n"
       << " " << std::setfill('-') << std::setw(totalw) << '-' << "\n" << std::setfill(' ');

    ss << std::setw(col1w) << "Mesh instances: " << std::setw(col2w) << mesh_instances.size()      << std::setw(col3w) << vec_size_str(mesh_instances)      << "\n"
       << std::setw(col1w) << "Emissives: "      << std::setw(col2w) << emissive_primitives.size() << std::setw(col3w) << vec_size_str(emissive_primitives) << "\n\n";

    ss << std::setw(titleoff - 4) << ' ' << "Materials" << "\n"
       << " " << std::setfill('-') << std::setw(totalw) << '-' << "\n" << std::setfill(' ');

    ss << std::setw(col1w) << "Mat. indices: " << std::setw(col2w) << material_indices.size() << std::setw(col3w) << vec_size_str(material_indices) << "\n"
       << std::setw(col1w) << "Mat. nodes: "   << std::setw(col2w) << material_nodes.size()   << std::setw(col3w) << vec_size_str(material_nodes)   << "\n\n";

    ss << std::setw(titleoff - 4) << ' ' << "Textures" << "\n"
       << " " << std::setfill('-') << std::setw(totalw) << '-' << "\n" << std::setfill(' ');

    ss << std::setw(col1w) << "Metadata: " << std::setw(col2w) << texture_metadata.size() << std::setw(col3w) << vec_size_str(texture_metadata) << "\n"
       << std::setw(col1w) << "Data: "     << std::setw(col2w) << texture_data.size()     << std::setw(col3w) << vec_size_str(texture_data)     << "\n\n";

    ss << " " << std::setfill('-') << std::setw(totalw) << '-' << "\n" << std::setfill(' ')
       << std::setw(col1w) << "Total: "     << std::setw(col2w) << ' ' << std::setw(col3w) << size_str(total_size) << "\n\n";

    return std::move(ss.str());
}

} } // namespace eclipse::scene

