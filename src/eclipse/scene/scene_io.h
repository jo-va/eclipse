#pragma once

#include <string>
#include <memory>

namespace eclipse {

class Resource;

namespace scene {

struct Scene;

std::unique_ptr<Scene> read(std::shared_ptr<Resource> resource);
void write(std::shared_ptr<Scene> scene, const std::string& file);

} } // namespace eclipse::scene
