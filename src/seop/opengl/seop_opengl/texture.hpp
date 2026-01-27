#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include <GL/glew.h>
namespace seop::opengl
{

class Texture_data{
  GLenum tex_tpye{GL_TEXTURE_2D};
};

class Gl_texture
{
  public:
    Gl_texture() = default;
    ~Gl_texture();

    void create();
    void bind(GLenum target);

    uint32_t id_{0};
    Texture_data data_;
};


} // namespace seop::primitive