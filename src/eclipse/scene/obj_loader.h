#pragma once

#include "eclipse/scene/raw_scene.h"
#include <memory>

namespace eclipse {

class Resource;

namespace scene {

std::unique_ptr<raw::Scene> load_obj(std::shared_ptr<Resource> scene);

} } // namespace eclipse::scene
