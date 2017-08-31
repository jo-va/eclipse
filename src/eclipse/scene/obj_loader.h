#pragma once

#include "eclipse/scene/raw_scene.h"
#include <memory>

namespace eclipse {

class Resource;

namespace obj {

std::unique_ptr<raw::Scene> load(std::shared_ptr<Resource> scene);

} } // namespace eclipse::obj
