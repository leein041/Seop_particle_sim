#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include <GL/glew.h>
namespace seop::opengl
{
class Gl_program
{
  public:
    Gl_program();
    ~Gl_program();

    uint32_t id_{0};
};



} // namespace seop::primitive