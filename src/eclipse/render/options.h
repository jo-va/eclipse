#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace eclipse { namespace render {

struct Options
{
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t num_bounces;
    uint32_t min_bounces_for_rr;
    uint32_t samples_per_pixel;
    float exposure;
    std::vector<std::string> device_blacklist;
    std::string force_primary_device;
};

} } // namespace eclipse::render
