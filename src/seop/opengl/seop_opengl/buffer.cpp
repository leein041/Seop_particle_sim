#include "buffer.hpp"

#include "seop_util/util.hpp"

#include <GL/glew.h>
#include <fstream>
#include <iostream>
#include <sstream>

namespace seop::opengl
{
Gl_buffer::Gl_buffer()
{
    if (id_ == 0) {
        glGenBuffers(1, &id_);
    }
}

Gl_buffer::~Gl_buffer()
{
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
    }
}

void Gl_buffer::bind(GLenum target)
{
    glBindBuffer(target, id_);
}

void Gl_buffer::bind_base(GLenum target, uint32_t slot)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, id_);
}

Frame_buffer::Frame_buffer()
{
}

Frame_buffer::~Frame_buffer()
{
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
    }
}

void Frame_buffer::create()
{
    if (id_ == 0) {
        glGenFramebuffers(1, &id_);
    }
}

void Frame_buffer::bind(GLenum target)
{
    glBindFramebuffer(target, id_);
}

void Frame_buffer::connect_tex(GLenum frame_target, GLenum slot, GLenum tex_target, uint32_t mipmap_level)
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_.id_, 0);
}

} // namespace seop::opengl