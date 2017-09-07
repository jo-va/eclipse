#pragma once

#include "eclipse/math/vec3.h"
#include "eclipse/math/mat4.h"

namespace eclipse { namespace scene {

struct Camera
{
    Vec3 eye;
    Vec3 look_at;
    Vec3 up;
    float pitch;
    float yaw;
    float fov;
    Mat4 view_mat;
    Mat4 proj_mat;
    bool invert_y;

    Camera(float fov = 45.0f);

    void update();
    void update_frustrum();
    void make_projection(float aspect);
};

} } // namespace eclipse::scene
