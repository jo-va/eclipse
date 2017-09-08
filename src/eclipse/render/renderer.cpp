#include "eclipse/render/renderer.h"
#include "eclipse/render/options.h"
#include "eclipse/scene/scene.h"

#include <memory>

namespace eclipse { namespace render {

Renderer::Renderer(std::shared_ptr<scene::Scene> scene, const Options& options)
    : m_scene(scene), m_options(options)
{

}

Renderer::~Renderer()
{

}

} } // namespace eclipse::render
