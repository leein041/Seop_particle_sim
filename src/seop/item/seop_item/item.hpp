#pragma once
#include "seop_opengl/buffer.hpp"
#include "seop_opengl/vertex_array.hpp"
#include "seop_primitive/vertex.hpp"

namespace seop::item
{
template <typename T>
class Render_item
{
  public:
    Render_item() = default;
    ~Render_item() = default;

    void                        create();
    void                        bind(GLenum target, GLenum usage);
    opengl::Gl_vertex_buffer<T> vb;
    opengl::Gl_vertex_array     va;
};

class Particle : public Render_item<primitive::Vertex_pcv>
{
  public:
    Particle();
    ~Particle() = default;
};

class Arrow_node : public Render_item<primitive::Vertex_pf>
{
  public:
    Arrow_node();
    ~Arrow_node() = default;
};

class Screen_quad : public Render_item<primitive::Vertex_pu>
{
  public:
    Screen_quad();
    ~Screen_quad() = default;
};

class Grid_quad : public Render_item<primitive::Vertex_pc>
{
  public:
    Grid_quad();
    ~Grid_quad() = default;
};

template <typename T>
inline void Render_item<T>::create()
{
    vb.buf_.create();
    va.create();
}

template <typename T>
inline void Render_item<T>::bind(GLenum target, GLenum usage)
{
    va.bind();
    vb.buf_.bind(target);
    vb.buffer_data(target, usage);
    T::specify_vertex_attribute();
}

} // namespace seop::item