#pragma once
#include "seop_math/math.hpp"

namespace seop::raytrace
{
class Ray
{
  public:
    math::Vec3 ori{math::Vec3::Zero};
    math::Vec3 dir{math::Vec3::Zero};
};

auto make_ray(const math::Vec2& ndc_pos, const math::Vec3& cam_pos, const math::Matrix& view,
              const math::Matrix& projection) -> Ray;
} // namespace seop::raytrace