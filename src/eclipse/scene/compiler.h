#pragma once

#include "eclipse/scene/scene.h"
#include "eclipse/scene/raw_scene.h"

#include <memory>
#include <cstdint>

namespace eclipse { namespace scene {

constexpr char SceneDiffuseMaterialName[] = "scene_diffuse_material";
constexpr char SceneEmissiveMaterialName[] = "scene_emissive_material";

constexpr uint32_t min_primitives_per_leaf = 10;

std::unique_ptr<Scene> compile(std::shared_ptr<raw::Scene> raw_scene);

} } // namespace eclipse::scene
