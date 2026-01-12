#pragma once
#include "seop_entity/particle.hpp"
#include "seop_input/input.hpp"
#include "seop_math/math.hpp"
#include "seop_scene/scene.hpp"
#include "shader.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>

namespace seop::graphic
{
enum Compute_type : uint32_t
{
    None = 0,
    Gravity = 1 << 1,
    Ampere = 1 << 2,
    End = 1 << 3,
};

struct Device_context
{
};

class Grid
{
  public:
    /*------------------
    | 3               2 |
    |                   |
    |                   |
    | 0               1 |
     ------------------*/
};

class Device final

{
  public:
    Device();
    ~Device() noexcept = default;

    void init();
    void compute(bool is_ui_hovered, float dt_f, scene::Scene_data &scene_data, input::Input_data &input_data);
    void prepare_grid(const math::Matrix &view, const math::Matrix &projection);
    void draw_grid();
    void prepare_draw(const math::Matrix &view, const math::Matrix &projection, float particle_size);
    void draw(size_t cnt);
    auto shut_down() -> bool;

    void clear_frame();
    void make_shader();
    void gen_buffer();
    void gen_shader_program();

    [[nodiscard]] auto compute_type() const -> uint32_t;

    void update_shader_buffer(GLuint buffer_id, GLuint binding_point, GLsizeiptr buffer_size, const void *data);
    void update_vertex_buffer(GLuint buffer_id, GLsizeiptr buffer_size, const void *data);

    GLuint particle_sb, ssbo_attractor;

    GLuint gravity_cs, ampere_cs;
    GLuint particle_vs, particle_fs;

    GLuint gravity_cs_program;
    GLuint ampere_cs_program;
    GLuint render_shader_program;

    // test
    math::Vec4 back_col_{0.1f,0.1f, 0.1f, 1.0f};
    std::vector<math::Vec4> grid_;
    GLuint grid_vs, grid_fs;
    GLuint grid_sb;
    GLuint grid_shader_program;

  private:
    GLuint make_cs(const char *src);
    GLuint make_vs(const char *src);
    GLuint make_fs(const char *src);

  private:
    Compute_type compute_type_{Compute_type::Gravity};

    // test
};

} // namespace seop::graphic

/*
 셰이더 순서
glCreateShader 
glShaderSource 
glCompileShader 
 프로그램 순서
glCreateProgram 
glAttachShader 
glLinkProgram 
glUseProgram 
*/