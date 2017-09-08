#include "eclipse/render/interactive_renderer.h"
#include "eclipse/scene/scene.h"
#include "eclipse/scene/camera.h"
#include "eclipse/render/options.h"

#include <memory>
#include <iostream>

namespace eclipse { namespace render {

InteractiveRenderer::InteractiveRenderer(std::shared_ptr<scene::Scene> scene, const Options& options)
{
    scene->camera.make_projection((float)options.frame_width / (float)options.frame_height);
    scene->camera.invert_y_axis(true);
}

int InteractiveRenderer::render()
{
    return 0;
}

} } // namespace eclipse::render
