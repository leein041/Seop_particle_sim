#pragma once
#include "item.hpp"
#include "seop_primitive/vertex.hpp"

namespace seop::item
{
class Wire_node : public Render_item<primitive::Vertex_pcs>
{
  public:
    Wire_node();
    ~Wire_node() = default;

    void  create();
    float line_width{1.0f};
};
} // namespace seop::item