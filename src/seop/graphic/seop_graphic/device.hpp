#pragma once
#include "vertex.hpp"

#include "seop_context/context.hpp"
#include "seop_entity/particle.hpp"
#include "seop_input/input.hpp"
#include "seop_math/math.hpp"
#include "seop_scene/scene.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>

namespace seop::msg
{
class Message;
}

namespace seop::graphic
{
enum Compute_type : uint32_t {
    None = 0,
    Gravity = 1 << 1,
    Electromagnetic = 1 << 2,
    End = 1 << 3,
};

class Shader_data
{
  public:
    GLuint gravity_cs, ampere_cs;
    GLuint particle_vs, particle_fs;
    GLuint grid_vs, grid_fs;
    GLuint quad_vs, quad_fs;
};

class Shader_buffer_data
{
  public:
    GLuint particle_sb;
    GLuint grid_sb;
};

class Shader_program_data
{
  public:
    GLuint gravity_cs_program;
    GLuint electro_cs_program;
    GLuint particle_shader_program;
    GLuint grid_shader_program;
    GLuint quad_shader_program;
};

class Screen_quad
{
  public:
    /*------------------
    | 3               2 |
    |                   |
    |                   |
    | 0               1 |
     ------------------*/
    math::Vec4 _0{-1.0f, 0.0f, -1.0f, 0.0f};
    math::Vec4 _1{1.0f, 0.0f, -1.0f, 0.0f};
    math::Vec4 _2{1.0f, 0.0f, 1.0f, 0.0f};
    math::Vec4 _3{-1.0f, 0.0f, 1.0f, 0.0f};
};

class Device_data
{
  public:
    float frame_rate{60.0f};
    float inv_frame_rate{1 / 60.0f};
};

class Device final
{
  public:
    Device();
    ~Device() noexcept = default;

    void init(Context& ctx);
    void clear_frame();
    auto shut_down() -> bool;
    void compute(float dt_f, Context& ctx);
    void render(Context& ctx);

    void prepare_grid(Context& ctx);
    void prepare_particle(Context& ctx);
    void draw_grid();
    void draw_particle(size_t cnt);
    auto on_message(msg::Message& ctx) -> bool;

    void set_draw_propeties();
    void bind_shader_buffer(GLuint buffer_id, GLuint binding_point, GLsizeiptr buffer_size, const void* data);

    [[nodiscard]] auto       data() const -> const Device_data&;
    [[nodiscard]] auto       data()  ->  Device_data&;
    [[nodiscard]] auto       grid() const -> const Screen_quad&;
    [[nodiscard]] auto       compute_type() const -> uint32_t;
    [[nodiscard]] auto       shader_data() const -> const Shader_data&;
    [[nodiscard]] auto       shader_buffer_data() const -> const Shader_buffer_data;
    [[nodiscard]] auto       shader_program_data() const -> const Shader_program_data;
    void                     set_compute_type(Compute_type type);

    // temp
    Vertex_buffer<Vertex_pu> quad_vb;
    Vertex_array<Vertex_pu>  quad_va;
    void                     create_vertex_buffer();
    void                     create_vertex_array();
    void                     prepare_quad(Context& ctx, GLuint tex_id);
    void                     draw_quad();
    float                    fade_scale{0.1f};
    math::Matrix             pre_view;

  private:
    void   create_shader();
    void   create_shader_buffer();
    void   create_shader_program();
    GLuint make_cs(const char* src);
    GLuint make_vs(const char* src);
    GLuint make_fs(const char* src);

    void   create_frame_buffer(GLuint& fbo, GLuint& tex, int width, int height);
    GLuint particle_layer_buffer;
    GLuint particle_layer_tex;

  private:
    Device_data         data_;
    Compute_type        compute_type_{Compute_type::Gravity};
    Screen_quad         grid_;
    math::Vec4          back_col_ = {0.0f, 0.0f, 0.0f, 1.0f};

    Shader_data         shader_data_;
    Shader_buffer_data  shader_buffer_data_;
    Shader_program_data shader_program_data_;
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