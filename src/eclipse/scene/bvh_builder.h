#pragma once

#include "eclipse/scene/bvh_node.h"
#include "eclipse/math/math.h"
#include "eclipse/math/vec3.h"
#include "eclipse/math/bbox.h"

#include <cstdint>
#include <vector>
#include <functional>

namespace eclipse { namespace bvh {

// Object must be a pointer to a type with the following methods:
//     BBox get_bbox();
//     Vec3 get_centroid();
//
// ScoringStrategy must provide the following static methods:
//     float score_split(const std::vector<Object>& items, uint8_t axis, float split_point, uint32_t* left_count, uint32_t* right_count)
//     float score_partition(const std::vector<Object>& items)
//
template <typename Object, typename ObjectAccesor, typename ScoringStrategy>
class Builder
{
public:
    typedef std::function<void(Node*, const std::vector<Object>&)> LeafCreationCallback;

    static const std::vector<Node> build(const std::vector<Object>& items, uint32_t min_leaf_size, LeafCreationCallback callback);

private:
    Builder(const std::vector<Object>& items)
        : m_callback(nullptr), m_min_leaf_size(0), m_num_partitioned_items(0), m_num_total_items(0)
        , m_num_nodes(0), m_num_leaves(0), m_max_depth(0), m_accessor(items)
    {
    }

    uint32_t partition(const std::vector<Object>& items, int depth);
    uint32_t create_leaf(Node* node, const std::vector<Object>& items);

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

    uint32_t m_min_leaf_size;
    uint32_t m_num_partitioned_items;
    uint32_t m_num_total_items;
    uint32_t m_num_nodes;
    uint32_t m_num_leaves;
    uint32_t m_max_depth;

    ObjectAccesor m_accessor;
};

template <typename Object, typename ObjectAccesor, typename ScoringStrategy>
const std::vector<Node> Builder<Object, ObjectAccesor, ScoringStrategy>::build(
        const std::vector<Object>& items, uint32_t min_leaf_size, LeafCreationCallback callback)
{
    Builder builder(items);
    builder.m_callback = callback;
    builder.m_min_leaf_size = min_leaf_size;
    builder.m_num_total_items = items.size();

    builder.partition(items, 0);

    return builder.m_nodes;
}

template <typename Object, typename ObjectAccesor, typename ScoringStrategy>
uint32_t Builder<Object, ObjectAccesor, ScoringStrategy>::partition(const std::vector<Object>& items, int depth)
{
    if (depth > (int)m_max_depth)
        m_max_depth = depth;

    Node node;

    // Calculate BBox for node
    for (auto& item : items)
        node.bbox.merge(m_accessor.get_bbox(item));

    if (items.size() <= m_min_leaf_size)
        return create_leaf(&node, items);

    // Get current node score
    float best_score = ScoringStrategy::score_partition(items, &m_accessor);

    constexpr size_t num_buckets = 100;
    static SplitScore score_list[3 * num_buckets];
    size_t num_scores = 0;

    const Vec3 side = node.bbox.pmax - node.bbox.pmin;

    for (uint8_t axis = 0; axis < 3; ++axis)
    {
        // Skip axis if bbox dimension is too small
        if (side[axis] < min_side_length)
            continue;

        const float split_step = side[axis] / (float)num_buckets;
        if (split_step < min_split_step)
            continue;

        // Compute split scores in parallel
#pragma omp parallel for
        for (size_t i = 0; i < num_buckets; ++i)
        {
            float split_point = node.bbox.pmin[axis] + i * split_step;
            SplitScore score;
            score.axis = axis;
            score.split_point = split_point;
            score.score = ScoringStrategy::score_split(items, &m_accessor, axis, split_point, &score.left_count, &score.right_count);
            score_list[num_scores++] = score;
        }
    }

    // Process all scores and pick the best split
    SplitScore* best_split = nullptr;
    for (size_t i = 0; i < num_scores; ++i)
    {
        SplitScore* score = &score_list[i];
        if (score->score < best_score)
        {
            best_score = score->score;
            best_split = score;
        }
    }

    // If we can't find a split that improves the current node score create a leaf
    if (best_split == nullptr)
        return create_leaf(&node, items);

    // Split items list into two sets
    std::vector<Object> left_items, right_items;
    left_items.reserve(best_split->left_count);
    right_items.reserve(best_split->right_count);

    for (auto& item : items)
    {
        Vec3 center = m_accessor.get_centroid(item);
        if (center[best_split->axis] < best_split->split_point)
            left_items.push_back(item);
        else
            right_items.push_back(item);
    }

    // Add node to list
    uint32_t node_index = m_nodes.size();
    m_nodes.push_back(node);
    ++m_num_nodes;

    // Partition children and update node indices
    uint32_t left_node_index = partition(left_items, depth + 1);
    uint32_t right_node_index = partition(right_items, depth + 1);
    m_nodes[node_index].set_child_nodes(left_node_index, right_node_index);

    return node_index;
}

template <typename Object, typename ObjectAccesor, typename ScoringStrategy>
uint32_t Builder<Object, ObjectAccesor, ScoringStrategy>::create_leaf(Node* node, const std::vector<Object>& items)
{
    m_callback(node, items);

    // Append node to list
    uint32_t node_index = m_nodes.size();
    m_nodes.push_back(*node);

    // Update stats
    ++m_num_leaves;
    m_num_partitioned_items += items.size();

    return node_index;
}

template <typename Object, typename ObjectAccesor>
class SAHStrategy
{
public:

    // Score a BVH split based on the surface area heuristic. The SAH calculates
    // the split score using the formulat:
    // left count * left BBox area + right count * right BBox area.
    // SAH avoids splits that generate empty partitions by assigning the worst
    // possible score when it encounters such cases.
    static float score_split(const std::vector<Object>& items, ObjectAccesor* accessor, uint8_t axis, float split_point, uint32_t* left_count, uint32_t* right_count)
    {
        BBox left_bbox, right_bbox;

        *left_count = 0;
        *right_count = 0;

        for (auto& object : items)
        {
            const Vec3 center = accessor->get_centroid(object);
            const BBox bbox = accessor->get_bbox(object);

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

        const Vec3 left_side = left_bbox.pmax - left_bbox.pmin;
        const Vec3 right_side = right_bbox.pmax - right_bbox.pmin;

        float score = (float)*left_count * (left_side.x * left_side.y + left_side.x * left_side.z + left_side.y * left_side.z) +
                      (float)*right_count * (right_side.x * right_side.y + right_side.x * right_side.z + right_side.y * right_side.z);

        return score;
    }

    // Calculate score for a partitioned list using formula:
    // count * BBox area
    // If the list is empty, then this method returns the worst possible score.
    static float score_partition(const std::vector<Object>& items, ObjectAccesor* accessor)
    {
        if (items.empty())
            return pos_inf;

        BBox bbox;
        for (auto& object : items)
            bbox.merge(accessor->get_bbox(object));

        const Vec3 side = bbox.pmax - bbox.pmin;

        return (float)items.size() * (side.x * side.y + side.x * side.z + side.y * side.z);
    }
};

} } // namespace eclipse::bvh
