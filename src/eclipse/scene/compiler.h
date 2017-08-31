#pragma once

#include "eclipse/scene/scene.h"
#include "eclipse/scene/raw_scene.h"
#include "eclipse/scene/bvh_builder.h"

namespace eclipse {

constexpr char SceneDiffuseMaterialName[] = "scene_diffuse_material";
constexpr char SceneEmissiveMaterialName[] = "scene_emissive_material";

class Compiler
{
public:
    Scene* compile(raw::Scene* raw_scene);
};

} // namespace eclipse
