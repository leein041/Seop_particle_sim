#include "camera.hpp"
#include "seop_math/math.hpp"

#include <cmath>

namespace seop::scene
{
Camera::Camera()
{
}

void Camera::init()
{
    make_view();
    make_projection();
}

void Camera::update()
{
    update_transform();
    make_view();
    make_projection();
}

void Camera::reset_state()
{
    Camera_state new_state;
    camera_state_ = new_state;
}

void Camera::make_view()
{
    math::Vec3 target = data_.transform.pos + data_.transform.forward; // XXX : why do I this?
    data_.view = math::Matrix::CreateLookAt(data_.transform.pos, target, data_.transform.up);
}

void Camera::make_projection()
{
    data_.projection = math::Matrix::CreatePerspective(data_.frustum.fov, data_.frustum.aspect, data_.frustum.near,
                                                       data_.frustum.far);
}

auto Camera::data() const -> const Camera_data&
{
    return data_;
}

auto Camera::data() -> Camera_data&
{
    return data_;
}

auto Camera::camera_state() const -> const Camera_state&
{
    return camera_state_;
}

auto Camera::camera_state() -> Camera_state&
{
    return camera_state_;
}

void Camera::set_transform(const Camera_trasform& tr)
{
    data_.transform = tr;
}

void Camera::set_aspect(float aspect)
{
    data_.frustum.aspect = aspect;
}

void Camera::update_transform()
{
    Camera_trasform& tr = data_.transform;

    math::Vec3       front;
    float            r_yaw = math::ToRadian(tr.yaw);
    float            r_pitch = math::ToRadian(tr.pitch);

    front.x_ = -cos(r_pitch) * sin(r_yaw); // NOTE : it's look -z dir
    front.y_ = sin(r_pitch);
    front.z_ = -cos(r_pitch) * cos(r_yaw); // NOTE : it's RH

    data_.transform.forward = front.Normalized();

    math::Vec3 worldUp{0.0f, 1.0f, 0.0f};
    tr.right = tr.forward.Cross(worldUp).Normalized();
    tr.up = tr.right.Cross(tr.forward).Normalized();
}

} // namespace seop::scene