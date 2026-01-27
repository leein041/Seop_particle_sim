#include "ray.hpp"
#include "seop_math/math.hpp"

namespace seop::raytrace
{
using namespace math;

auto make_ray(const math::Vec2& ndc_pos, const math::Vec3& cam_pos, const math::Matrix& view,
              const math::Matrix& projection) -> Ray
{
    Vec4   clip_space{ndc_pos.x_, ndc_pos.y_, -1.0f, 1.0f};
    Matrix view_inv = view.Invert();
    Matrix projection_inv = projection.Invert();

    Vec4   view_space = projection_inv * clip_space;
    view_space.z_ = -1.0f; // 오른손 좌표계
    view_space.w_ = 0.0f;  // 방향 벡터이므로 w를 0으로 설정
    Vec3 ray_ori = cam_pos;
    Vec3 ray_dir = (view_inv * view_space).xyz().Normalized();

    return raytrace::Ray{ray_ori, ray_dir};
}

} // namespace seop::raytrace
