#pragma once
#include "seop_math/math.hpp"

#include <GL/glew.h>
#include <vector>

namespace seop::graphic
{
class Vertex_pc
{
  public:
    math::Vec4 pos{math::Vec4::Zero};
    math::Vec4 col{math::Vec4::Zero};
};

class Vertex_pu
{
  public:
    math::Vec4 pos{math::Vec4::Zero};
    math::Vec4 uv{math::Vec4::Zero};
};

template <typename T>
class Vertex_buffer // = buffer
{
  public:
    std::vector<T> vertices_;
    GLuint         id_;
};

template <typename T>
class Vertex_array // = input layout
{
  public:
    GLuint id_;
};
} // namespace seop::graphic