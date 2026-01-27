#include "texture.hpp"

namespace seop::opengl
{

Gl_texture::~Gl_texture()
{
    if (id_ != 0) {
        glDeleteTextures(1, &id_);
    }
}

void Gl_texture::create()
{
    glGenTextures(1, &id_);
}

void Gl_texture::bind(GLenum target)
{
    glBindTexture(target, id_);
}

} // namespace seop::opengl