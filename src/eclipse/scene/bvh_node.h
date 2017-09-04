#pragma once

#include "eclipse/math/bbox.h"

#include <cstdint>

namespace eclipse { namespace bvh {

struct Node
{
    BBox bbox;
    int32_t left_data;
    int32_t right_data;

    void set_mesh_index(uint32_t index)
    {
        left_data = -int32_t(index);
    }

    uint32_t get_mesh_index() const
    {
        return uint32_t(-left_data);
    }

    void set_primitives(uint32_t first, uint32_t num)
    {
        left_data = -int32_t(first);
        right_data = int32_t(num);
    }

    uint32_t get_primitives_offset() const
    {
        return uint32_t(-left_data);
    }

    uint32_t get_num_primitives() const
    {
        return uint32_t(right_data);
    }

    void offset_child_nodes(int32_t offset)
    {
        // Ignore leaves
        if (left_data <= 0)
            return;

        left_data += offset;
        right_data += offset;
    }

    void set_child_nodes(uint32_t left, uint32_t right)
    {
        left_data = int32_t(left);
        right_data = int32_t(right);
    }
};

} } // namespace eclipse::bvh
