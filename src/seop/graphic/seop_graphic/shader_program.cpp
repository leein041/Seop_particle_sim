#include "shader_program.hpp"

#include "seop_opengl/shader.hpp"

namespace seop::graphic
{
Shader_program::Shader_program()
{
}

Shader_program::~Shader_program()
{
}

void Shader_program::create()
{
    program_.id_ = glCreateProgram();
}

void Shader_program::attach(opengl::Gl_shader&& shader)
{
    glAttachShader(program_.id_, shader.id_);
}

void Shader_program::link()
{
    glLinkProgram(program_.id_);
}

void Shader_program::compute(uint32_t work_x,uint32_t work_y,uint32_t work_z)
{
    glDispatchCompute(work_x, work_y, work_z);
}

void Shader_program::set_uniform(Context& ctx)
{
    uniform_setter_(ctx);
}

void Shader_program::set_uniform_setter(Uniform_setter func)
{
    uniform_setter_ = [this, func](Context& ctx) { func(program_.id_, ctx); };
}
} // namespace seop::graphic