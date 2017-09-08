#pragma once

#include "eclipse/scene/scene.h"
#include "eclipse/render/options.h"

#include <memory>

namespace eclipse { namespace render {

class InteractiveRenderer
{
public:
    InteractiveRenderer(std::shared_ptr<scene::Scene> scene, const Options& options);

    int render();
};

} } // namespace eclipse::render
