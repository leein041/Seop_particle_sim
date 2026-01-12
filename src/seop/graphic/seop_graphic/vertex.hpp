#pragma once
#include "seop_math/math.hpp"

namespace seop::graphic
{
class Vertex_pc
{
  public:
    math::Vec4 pos{math::Vec4::Zero};
    math::Vec4 col{math::Vec4::Zero};
};
} // namespace seop::graphic