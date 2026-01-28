#include "vertex_array.hpp"

#include <GL/glew.h>

namespace seop::opengl
{
Gl_vertex_array::Gl_vertex_array()
{
    if (id_ == 0) {
        glGenVertexArrays(1, &id_);
    }
}

Gl_vertex_array::~Gl_vertex_array()
{
    if (id_ != 0) {
        glDeleteVertexArrays(1, &id_);
    }
}

void Gl_vertex_array::bind()
{
    glBindVertexArray(id_);
}
} // namespace seop::opengl