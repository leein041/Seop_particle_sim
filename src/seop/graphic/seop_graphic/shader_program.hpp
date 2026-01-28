#pragma once

#include "seop_opengl/program.hpp"

#include <cstdint>
#include <functional>

namespace seop
{
class Context;
}
namespace seop::opengl{
  class Gl_shader;
}
namespace seop::graphic
{
using Uniform_setter = std::function<void(uint32_t, Context&)>;

class Shader_program
{
  public:
    Shader_program();
    ~Shader_program();

    void attach(opengl::Gl_shader&& shader);
    void link();
    void compute(uint32_t work_x, uint32_t work_y, uint32_t work_z);

    void set_uniform(Context& ctx);
    void set_uniform_setter(Uniform_setter func);

  private:
    opengl::Gl_program         program_;
    std::function<void(Context&)> uniform_setter_;
};
} // namespace seop::graphic