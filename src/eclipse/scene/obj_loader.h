#pragma once

#include "eclipse/util/except.h"

#include <memory>
#include <string>

namespace eclipse {

class Resource;

namespace raw {
    struct Scene;
}

namespace scene {

class ObjError : public Error
{
public:
    ObjError(const std::string& msg) : Error(msg) { }
};

std::unique_ptr<raw::Scene> load_obj(std::shared_ptr<Resource> scene);

} } // namespace eclipse::scene
