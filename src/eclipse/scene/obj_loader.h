#pragma once

#include <memory>

namespace eclipse {

class Resource;

namespace raw {
    struct Scene;
}

namespace scene {

std::unique_ptr<raw::Scene> load_obj(std::shared_ptr<Resource> scene);

} } // namespace eclipse::scene
