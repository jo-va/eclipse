#pragma once

#include "eclipse/render/options.h"
#include "eclipse/tracer/tracer.h"
#include "eclipse/scene/scene.h"

#include <cstdint>
#include <vector>
#include <memory>

namespace eclipse { namespace render {

class Renderer
{
public:
    Renderer(std::shared_ptr<scene::Scene> scene, const Options& options);
    virtual ~Renderer();

protected:
    std::shared_ptr<scene::Scene> m_scene;
    Options m_options;
    std::vector<Tracer> m_tracers;
    std::vector<uint32_t> m_blocks;
};

} } // namespace eclipse::render
