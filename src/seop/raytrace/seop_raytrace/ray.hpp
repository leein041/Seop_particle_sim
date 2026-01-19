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
} // namespace seop::raytrace