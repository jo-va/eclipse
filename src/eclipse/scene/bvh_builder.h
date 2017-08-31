#pragma once

#include "eclipse/math/math.h"
#include "eclipse/math/vec3.h"
#include "eclipse/math/bbox.h"
#include "eclipse/util/thread_pool.h"

#include <cstdint>
#include <vector>
#include <future>

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

//
// Object must provide the following methods:
//     BBox get_bbox();
//     Vec3 get_centroid();
//
// ScoringStrategy must provide the following static methods:
//     float score_split(const std::vector<Object*>& items, uint8_t axis, float split_point, uint32_t* left_count, uint32_t* right_count)
//     float score_partition(const std::vector<Object*>& items)
//
template <typename Object, typename ScoringStrategy>
class Builder
{
public:
    typedef void (*LeafCreationCallback)(Node* leaf, const std::vector<Object*>& items);

    static const std::vector<Node>& build(const std::vector<Object*>& items, LeafCreationCallback callback, uint32_t min_leaf_size);

private:
    Builder()
        : m_callback(nullptr), m_min_leaf_size(0), m_num_partitioned_items(0), m_num_total_items(0)
        , m_num_nodes(0), m_num_leaves(0), m_max_depth(0)
    {
    }

    uint32_t partition(const std::vector<Object*>& items, int depth);
    uint32_t create_leaf(Node* node, const std::vector<Object*>& items);

private:

    struct SplitScore
    {
        uint8_t axis;
        float split_point;
        uint32_t left_count, right_count;
        float score;
    };

    static constexpr float min_side_length = 1e-3f;
    static constexpr float min_split_step = 1e-5f;

    std::vector<Node> m_nodes;

    LeafCreationCallback m_callback;

    mutable ThreadPool<SplitScore> m_thread_pool;

    uint32_t m_min_leaf_size;
    uint32_t m_num_partitioned_items;
    uint32_t m_num_total_items;
    uint32_t m_num_nodes;
    uint32_t m_num_leaves;
    uint32_t m_max_depth;
};

template <typename Object, typename ScoringStrategy>
const std::vector<Node>& Builder<Object, ScoringStrategy>::build(const std::vector<Object*>& items, LeafCreationCallback callback, uint32_t min_leaf_size)
{
    Builder builder;
    builder.m_callback(callback);
    builder.m_min_leaf_size = min_leaf_size;
    builder.m_num_total_items = items.size();

    builder.partition(items, 0);

    return builder.m_nodes;
}

template <typename Object, typename ScoringStrategy>
uint32_t Builder<Object, ScoringStrategy>::partition(const std::vector<Object*>& items, int depth)
{
    if (depth > m_max_depth)
        m_max_depth = depth;

    Node node;

    // Calculate BBox for node
    for (auto item : items)
        node.bbox.merge(item->get_bbox());

    if (items.size() <= m_min_leaf_size)
        return create_leaf(&node, items);

    // Get current node score
    float best_score = ScoringStrategy::score_partition(items);

    m_thread_pool.set_sleep_time(0);
    std::vector<std::future<SplitScore>> jobs;
    jobs.reserve((uint32_t)(1024.0f / (float)(depth + 1) * 3));

    Vec3 side = node.bbox.pmax - node.bbox.pmin;

    for (uint8_t axis = 0; axis < 3; ++axis)
    {
        if (side[axis] < min_side_length)
            continue;

        float split_step = side[axis] / (1024.0f / (float)(depth + 1));
        if (split_step < min_split_step)
            continue;

        for (float split_point = node.bbox.pmin[axis]; split_point < node.bbox.pmax[axis]; split_point += split_step)
        {
            jobs.push_back(std::move(m_thread_pool.submit(([this, items, axis, split_point]() -> SplitScore
            {
                SplitScore score;
                score.axis = axis;
                score.split_point = split_point;
                score.score = ScoringStrategy::score_split(items, axis, split_point, &score.left_count, &score.right_count);

                return score;
            }))));
        }
    }

    for (auto& job : jobs)
        job.wait();

    SplitScore* best_split = nullptr;

    for (size_t i = 0; i < jobs.size(); ++i)
    {
        SplitScore score = jobs[i];
        if (score.score < best_score)
        {
            best_score = score.score;
            best_split = &score;
        }
    }

    if (best_split == nullptr)
        return create_leaf(&node, items);

    std::vector<Object*> left_items, right_items;
    for (auto item : items)
    {
        Vec3 center = item->get_centroid();
        if (center[best_split->axis] < best_split.split_point)
            left_items.push_back(item);
        else
            right_items.push_back(item);
    }

    uint32_t node_index = m_nodes.size();
    m_nodes.push_back(node);
    ++m_num_nodes;

    uint32_t left_node_index = partition(left_items, depth + 1);
    uint32_t right_node_index = partition(right_items, depth + 1);
    m_nodes[node_index].set_child_nodes(left_node_index, right_node_index);

    return node_index;
}

template <typename Object, typename ScoringStrategy>
uint32_t Builder<Object, ScoringStrategy>::create_leaf(Node* node, const std::vector<Object*>& items)
{
    m_callback(node, items);

    uint32_t node_index = m_nodes.size();
    m_nodes.push_back(*node);

    ++m_num_leaves;
    m_num_partitioned_items += items.size();

    return node_index;
}

template <typename Object>
class SAHStrategy
{
public:
    static float score_split(const std::vector<Object*>& items, uint8_t axis, float split_point, uint32_t* left_count, uint32_t* right_count)
    {
        BBox left_bbox, right_bbox;

        *left_count = 0;
        *right_count = 0;

        for (auto object : items)
        {
            Vec3 center = object->get_centroid();
            BBox bbox = object->get_bbox();

            if (center[axis] < split_point)
            {
                ++*left_count;
                left_bbox.merge(bbox);
            }
            else
            {
                ++*right_count;
                right_bbox.merge(bbox);
            }
        }

        if (*left_count == 0 || *right_count == 0)
            return pos_inf;

        Vec3 left_side = left_bbox.pmax - left_bbox.pmin;
        Vec3 right_side = right_bbox.pmax - right_bbox.pmin;

        float score = *left_count * (left_side.x * left_side.y + left_side.x * left_side.z + left_side.y * left_side.z) +
                      *right_count * (right_side.x * right_side.y + right_side.x * right_side.z + right_side.y * right_side.z);

        return score;
    }

    static float score_partition(const std::vector<Object*>& items)
    {
        if (items.empty())
            return pos_inf;

        BBox bbox;
        for (auto object : items)
            bbox.merge(object->get_bbox());

        Vec3 side = bbox.pmax - bbox.pmin;

        return (float)items.size() * (side.x * side.y + side.x * side.z + side.y * side.z);
    }
};

} } // namespace eclipse::bvh
