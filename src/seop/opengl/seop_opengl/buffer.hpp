#pragma once
#include "texture.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include <GL/glew.h>

namespace seop::opengl
{
class Gl_buffer
{
  public:
    Gl_buffer() = default;
    ~Gl_buffer();

    void     create();
    void     bind(GLenum target);
    void     bind_base(GLenum target, uint32_t slot);
    uint32_t id_{0};
};

class Frame_buffer
{
  public:
    void       create();
    void       bind(GLenum target);
    void       connect_tex(GLenum frame_target, GLenum slot, GLenum tex_target, uint32_t mipmap_level);
    bool       use_tex{false};
    uint32_t   id_{0};
    Gl_texture tex_;
};

template <typename T>
class Gl_vertex_buffer
{
  public:
    Gl_vertex_buffer() = default;
    ~Gl_vertex_buffer() = default;
    void           use_shader_buffer(GLenum target, GLenum usage, uint32_t slot);
    void           buffer_data(GLenum target, GLenum usage);
    std::vector<T> vertices;
    Gl_buffer      buf_;
};

template <typename T>
inline void Gl_vertex_buffer<T>::use_shader_buffer(GLenum target, GLenum usage, uint32_t slot)
{
    buf_.bind(target);
    buffer_data(target, usage);
    buf_.bind_base(target, slot);
}

template <typename T>
inline void Gl_vertex_buffer<T>::buffer_data(GLenum target, GLenum usage)
{
    glBufferData(target, vertices.size() * sizeof(T), vertices.data(), usage);
}

} // namespace seop::opengl