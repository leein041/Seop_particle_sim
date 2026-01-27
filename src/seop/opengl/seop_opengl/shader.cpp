#include "shader.hpp"

#include <GL/glew.h>
#include <fstream>
#include <iostream>
#include <sstream>
namespace seop::opengl
{
Gl_shader::Gl_shader(const std::string& path, GLenum shader_type)
{
    std::string shader_source = load_source(path);
    const char* shader_ptr = shader_source.c_str();

    id_ = glCreateShader(shader_type);
    glShaderSource(id_, 1, &shader_ptr, NULL);
    glCompileShader(id_);
}

Gl_shader::~Gl_shader()
{
    if (id_ != 0) {
        glDeleteShader(id_);
    }
}

std::string Gl_shader::load_source(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "There is not file : " << path << std::endl;
        return 0;
    }
    std::stringstream buf;
    buf << file.rdbuf();
    std::string shader_source = buf.str();
    return shader_source;
}
} // namespace seop::opengl