#include "wire.hpp"

namespace seop::item
{
Wire_node::Wire_node()
{
}

void Wire_node::create()
{
    vb.vertices.reserve(100);
    Render_item::create();
}
} // namespace seop::item