#pragma once

#include <cstdint>

namespace seop::opengl
{
class Gl_vertex_array
{
  public:
    Gl_vertex_array();
    ~Gl_vertex_array();

    void bind();
    uint32_t id_{0};
};
} // namespace seop::opengl