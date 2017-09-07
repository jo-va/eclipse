#include "eclipse/scene/scene.h"
#include "eclipse/scene/bvh_node.h"

#include <istream>
#include <ostream>

namespace eclipse { namespace scene {

std::istream& operator>>(std::istream& is, MeshInstance& m)
{
    is >> m.mesh_index;
    is >> m.bvh_root;
    is >> m.padding[0];
    is >> m.padding[1];
    is >> m.transform;
    return is;
}

std::ostream& operator<<(std::ostream& os, const MeshInstance& m)
{
    os << m.mesh_index;
    os << m.bvh_root;
    os << m.padding[0];
    os << m.padding[1];
    os << m.transform;
    return os;
}

std::istream& operator>>(std::istream& is, EmissivePrimitive& e)
{
    is >> e.transform;
    is >> e.area;
    is >> e.primitive_index;
    is >> e.material_index;
    is >> e.type;
    return is;
}

std::ostream& operator<<(std::ostream& os, const EmissivePrimitive& e)
{
    os << e.transform;
    os << e.area;
    os << e.primitive_index;
    os << e.material_index;
    os << e.type;
    return os;
}

std::istream& operator>>(std::istream& is, TextureMetadata& m)
{
    uint32_t format;
    is >> format;
    m.format = format;
    is >> m.width;
    is >> m.height;
    is >> m.offset;
    return is;
}

std::ostream& operator<<(std::ostream& os, const TextureMetadata& m)
{
    os << (uint32_t)m.format;
    os << m.width;
    os << m.height;
    os << m.offset;
    return os;
}

std::istream& operator>>(std::istream& is, Scene& s)
{
    size_t num;
    is >> num;
    for (size_t i = 0; i < num; ++i)
    {
        bvh::Node node;
        is >> node;
        s.bvh_nodes.push_back(node);
    }
    is >> num;
    for (size_t i = 0; i < num; ++i)
    {
        MeshInstance inst;
        is >> inst;
        s.mesh_instances.push_back(inst);
    }
    is >> num;
    for (size_t i = 0; i < num; ++i)
    {
        material::Node node;
        is >> node;
        s.material_nodes.push_back(node);
    }
    is >> num;
    for (size_t i = 0; i < num; ++i)
    {
        EmissivePrimitive eprim;
        is >> eprim;
        s.emissive_primitives.push_back(eprim);
    }
    is >> num;
    for (size_t i = 0; i < num; ++i)
    {
        uint8_t data;
        is >> data;
        s.texture_data.push_back(data);
    }
    is >> num;
    for (size_t i = 0; i < num; ++i)
    {
        TextureMetadata meta;
        is >> meta;
        s.texture_metadata.push_back(meta);
    }
    is >> num;
    for (size_t i = 0; i < num; ++i)
    {
        Vec4 v;
        is >> v;
        s.vertices.push_back(v);
    }
    is >> num;
    for (size_t i = 0; i < num; ++i)
    {
        Vec4 n;
        is >> n;
        s.normals.push_back(n);
    }
    is >> num;
    for (size_t i = 0; i < num; ++i)
    {
        Vec2 uv;
        is >> uv;
        s.uvs.push_back(uv);
    }
    is >> num;
    for (size_t i = 0; i < num; ++i)
    {
        uint32_t index;
        is >> index;
        s.material_indices.push_back(index);
    }
    is >> s.scene_diffuse_mat_index;
    is >> s.scene_emissive_mat_index;
    is >> s.camera;
    return is;
}

std::ostream& operator<<(std::ostream& os, const Scene& s)
{
    os << s.bvh_nodes.size();
    for (size_t i = 0; i < s.bvh_nodes.size(); ++i)
        os << s.bvh_nodes[i];
    os << s.mesh_instances.size();
    for (size_t i = 0; i < s.mesh_instances.size(); ++i)
        os << s.mesh_instances[i];
    os << s.material_nodes.size();
    for (size_t i = 0; i < s.material_nodes.size(); ++i)
        os << s.material_nodes[i];
    for (size_t i = 0; i < s.emissive_primitives.size(); ++i)
        os << s.emissive_primitives[i];
    os << s.texture_data.size();
    for (size_t i = 0; i < s.texture_data.size(); ++i)
        os << s.texture_data[i];
    os << s.texture_metadata.size();
    for (size_t i = 0; i < s.texture_metadata.size(); ++i)
        os << s.texture_metadata[i];
    os << s.vertices.size();
    for (size_t i = 0; i < s.vertices.size(); ++i)
        os << s.vertices[i];
    os << s.normals.size();
    for (size_t i = 0; i < s.normals.size(); ++i)
        os << s.normals[i];
    os << s.uvs.size();
    for (size_t i = 0; i < s.uvs.size(); ++i)
        os << s.uvs[i];
    os << s.scene_diffuse_mat_index;
    os << s.scene_emissive_mat_index;
    os << s.camera;
    return os;
}

} } // namespace eclipse::scene

