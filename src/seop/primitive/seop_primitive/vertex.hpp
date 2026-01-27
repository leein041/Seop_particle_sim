#pragma once
#include "seop_math/math.hpp"

#include <cstdint>
#include <vector>

namespace seop::primitive
{

static uint32_t vert_id = 0;

enum Vertex_state : uint16_t {
    STATE_NONE = 0,
    STATE_HOVER_ONLY = 1 << 1,
    STATE_HOVER_WIRE = 1 << 2,
    STATE_SELECTED = 1 << 3,
    STATE_HOLD = 1 << 4,
    STATE_INACTIVE = 1 << 5,
    STATE_DESTROYED = 1 << 6,
};

class Vertex_p
{
  public:
    math::Vec4 pos{math::Vec4::Zero};
};

class Vertex_pc
{
  public:
    static void specify_vertex_attribute();
    math::Vec4 pos{math::Vec4::Zero};
    math::Vec4 col{math::Vec4::Zero};
};

class Vertex_pcv
{
  public:
    static void specify_vertex_attribute();

    math::Vec4  pos{math::Vec4::Zero};
    math::Vec4  col{math::Vec4::Zero};
    math::Vec4  vel{math::Vec4::Zero};
};

class Vertex_pcs
{
  public:
    static void specify_vertex_attribute();
    math::Vec4  pos{math::Vec4::Zero};

    math::Vec4  col{math::Vec4::Zero};

    float       size{1.0f};
    float       coord{0.0f};
    uint32_t    id{0};

    uint16_t    state{0};
    uint16_t    pad;
};

class Vertex_pu
{
  public:
    static void specify_vertex_attribute();
    math::Vec4 pos{math::Vec4::Zero};
    math::Vec4 uv{math::Vec4::Zero};
};

class Vertex_pf // 화살표 전용 정점
{
  public:
    static void specify_vertex_attribute();
    math::Vec4  pos{math::Vec4::Zero};
    math::Vec4  field{math::Vec4::Zero}; // xyz : 위치, w : 크기
};

} // namespace seop::primitive