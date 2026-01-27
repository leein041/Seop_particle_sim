#include "program.hpp"

namespace seop::opengl
{

Gl_program::Gl_program()
{
}

Gl_program::~Gl_program()
{
    if (id_ != 0) {
        glDeleteProgram(id_);
    }
}
} // namespace seop::opengl