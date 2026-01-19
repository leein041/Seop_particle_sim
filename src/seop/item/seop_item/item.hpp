#pragma once
#include "seop_primitive/vertex.hpp"

namespace seop::item
{
template <typename T>
class Render_item
{
  public:
    Render_item() {};
    ~Render_item() = default;
    primitive::Vertex_buffer<T> vb;
    primitive::Vertex_array<T>  va;
};

//temp
class Square_wire : public Render_item<primitive::Vertex_p>
{
  public:
    Square_wire();
    ~Square_wire() = default;

    float      line_width{1.0f};
    math::Vec2 scale{1.0f, 1.0f};
};
class Wire : public Render_item<primitive::Vertex_pcs>
{
  public:
    Wire();
    ~Wire() = default;

    float      line_width{1.0f};
};
} // namespace seop::item