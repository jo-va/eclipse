#pragma once

#include <cstdint>

namespace eclipse {

struct TraceTile
{
    uint32_t frame_width;
    uint32_t frame_height;

    uint32_t tile_x;
    uint32_t tile_y;
    uint32_t tile_w;
    uint32_t tile_h;

    uint32_t samples_per_pixel;
    uint32_t bounces_before_russian_roulette;
    uint32_t accumulated_samples;
    float exposure;
};

struct TracerStats
{
    uint32_t tile_w;
    uint32_t tile_h;

    float update_time_ms;
    float render_time_ms;
};

enum TracerFlags
{
    LOCAL  = 1 << 0,
    REMOTE = 1 << 1,
    CPU    = 1 << 2,
    GPU    = 1 << 3
};

enum UpdateType
{
    FrameDimensions,
    SceneData,
    CameraData
};

class Tracer
{
public:
    virtual const char* get_name() const = 0;
    virtual float get_gflops_estimate() const = 0;
    virtual uint32_t get_flags() const = 0;
    virtual const TracerStats* get_stats() const = 0;

    virtual void init() = 0;
    virtual void terminate() = 0;

    virtual void update(UpdateType type, void* data, bool synchronous = true) = 0;
    virtual void trace(const TraceTile* tile) = 0;
    virtual void merge_output(const Tracer* tracer, const TraceTile* tile) = 0;
    virtual void sync_framebuffer(const TraceTile* tile) = 0;
};

} // namespace eclipse
