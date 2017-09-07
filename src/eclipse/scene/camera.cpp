#include "eclipse/scene/camera.h"
#include "eclipse/math/vec3.h"
#include "eclipse/math/mat4.h"
#include "eclipse/math/quaternion.h"

namespace eclipse { namespace scene {

Camera::Camera(float fov)
    : eye(0, 0, 0), look_at(0, 0, -1), up(0, 1, 0)
    , pitch(0), yaw(0)
    , fov(fov)
    , invert_y(false)
{
}

void Camera::make_projection(float aspect)
{
    proj_mat = make_perspective(fov, aspect, 1.0f, 1000.0f);
    update();
}

void Camera::update()
{
    Vec3 dir = normalize(look_at - eye);
    Vec3 pitch_axis = cross(dir, up);
    Quaternion pitch_quat = quat_from_axis_angle(pitch_axis, pitch);
    Quaternion yaw_quat = quat_from_axis_angle(up, yaw);
    Quaternion orient_quat = normalize(pitch_quat * yaw_quat);

    dir = rotate_vector(orient_quat, dir);
    look_at = eye + dir;

    view_mat = make_look_at(eye, look_at, up);
    update_frustrum();
}

void Camera::update_frustrum()
{

}

} } // namespace eclipse::scene
