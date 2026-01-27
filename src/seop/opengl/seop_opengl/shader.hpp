#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include <GL/glew.h>
namespace seop::opengl
{

class Gl_shader
{
  public:
    Gl_shader(const std::string& path, GLenum shader_type);
    Gl_shader(const Gl_shader&) = delete;
    Gl_shader& operator=(const Gl_shader&) = delete;
    ~Gl_shader();

    std::string load_source(const std::string& path);
    uint32_t id_{0};
};


} // namespace seop::primitive