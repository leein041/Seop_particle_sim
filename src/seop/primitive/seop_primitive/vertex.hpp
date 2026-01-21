#pragma once
#include "seop_math/math.hpp"

#include <vector>

namespace seop::primitive
{

class Vertex_p
{
  public:
    math::Vec4 pos{math::Vec4::Zero};
};

class Vertex_pc
{
  public:
    math::Vec4 pos{math::Vec4::Zero};
    math::Vec4 col{math::Vec4::Zero};
};

static uint32_t vert_id = 0;

class Vertex_pcs
{
  public:
    math::Vec4 pos{math::Vec4::Zero};
    math::Vec4 col{math::Vec4::Zero};
    float      size{1.0f};
    float      coord{0.0f};
    uint32_t   id{0};
    uint16_t   state{0};
    uint16_t   pad;
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
    std::vector<T> vertices;
    uint32_t       id;
};

template <typename T>
class Vertex_array // = input layout
{
  public:
    uint32_t id;
};
} // namespace seop::primitive