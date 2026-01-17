#pragma once
#include "vertex.hpp"

#include "seop_context/context.hpp"
#include "seop_entity/particle.hpp"
#include "seop_input/input.hpp"
#include "seop_math/math.hpp"
#include "seop_scene/scene.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <array>
#include <functional>
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

class Device_data
{
  public:
    float        fade_scale{0.1f};
    float        frame_rate{60.0f};
    float        inv_frame_rate{1 / 60.0f};
    math::Vec4   back_col = {0.0f, 0.0f, 0.0f, 1.0f};
    Compute_type compute_type{Compute_type::Electromagnetic};
};

// new
using Uniform_setter = std::function<void(Context&)>;
enum class Shader_task_type {
    Compute_gravity,
    Compute_electromagnetic,
    Compute_magnetic_field,

    Render_particle,
    Render_screen_quad,
    Render_grid,
    End,
};
enum class Shader_buffer_type {
    Particle,
    End,
};
enum class Frame_buffer_type {
    Particle_layer,
    End,
};
constexpr size_t FRAME_BUFFER_MAX = static_cast<size_t>(Frame_buffer_type::End);
constexpr size_t SHADER_TASK_MAX = static_cast<size_t>(Shader_task_type::End);
constexpr size_t SHADER_BUFFER_MAX = static_cast<size_t>(Shader_buffer_type::End);

template <typename T>
class Quad
{
  public:
    std::vector<T> vertices;
    GLuint         vb;
    GLuint         va;
};

class Shader_task
{
  public:
    GLuint         program{0};
    Uniform_setter uniform_setter;
};

class Shader_buffer
{
  public:
    GLuint sb;
};

class Frame_buffer
{
  public:
    bool   use_tex{false};
    GLuint buffer;
    GLuint texture;
};

class Device final
{
  public:
    Device();
    ~Device() noexcept = default;

    // temp helper
    template <typename T>
    constexpr size_t to_idx(T e)
    {
        return static_cast<size_t>(e);
    }

    void init(Context& ctx);
    void clear_frame();
    void shut_down();
    void compute(Context& ctx);
    void render(Context& ctx);
    auto on_message(msg::Message& msg) -> bool;

    void prepare_grid(Context& ctx);
    void prepare_particle(Context& ctx);
    void prepare_quad(Context& ctx);
    void draw_quad();
    void draw_grid();
    void draw_particle(size_t cnt);

    void init_draw_propeties();
    void update_shader_buffer(GLuint buffer_id, GLuint binding_point, GLsizeiptr buffer_size, const void* data);

    [[nodiscard]] auto data() const -> const Device_data&;
    [[nodiscard]] auto data() -> Device_data&;
    void               set_fade_scale(float scale);
    void               set_frame_rate(float rate);
    void               set_back_col(const math::Vec4& col);
    void               set_compute_type(Compute_type type);
    
  private:
    void   add_shader_buffer(Shader_buffer_type type);
    void   add_compute_task(Shader_task_type type, const char* cs_src,
                            std::function<void(GLuint, Context&)> uniform_setter);
    void   add_render_task(Shader_task_type type, const char* vs_src, const char* fs_src,
                           std::function<void(GLuint, Context&)> uniform_setter);
    void   add_frame_buffer(Frame_buffer_type type, int width, int height);

    void   create_screen_qaud();
    void   create_grid();
    GLuint create_cs(const char* src);
    GLuint create_vs(const char* src);
    GLuint create_fs(const char* src);

    GLuint particle_layer_buffer;
    GLuint particle_layer_tex;

  private:
    Device_data                                  data_;
    std::array<Shader_buffer, SHADER_BUFFER_MAX> shader_buffers_;
    std::array<Shader_task, SHADER_TASK_MAX>     shader_tasks_;
    std::array<Frame_buffer, FRAME_BUFFER_MAX>   frame_buffers_;
    Quad<Vertex_pu>                              screen_quad_;
    Quad<Vertex_pc>                              grid_quad_;
};
} // namespace seop::graphic