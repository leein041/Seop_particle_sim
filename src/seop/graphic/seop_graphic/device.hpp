#pragma once
#include "shader_program.hpp"

#include "seop_item/item.hpp"
#include "seop_opengl/buffer.hpp"
#include "seop_opengl/shader.hpp"
#include "seop_opengl/texture.hpp"
#include "seop_primitive/vertex.hpp"

#include "seop_math/math.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace seop
{
class Context;
}

namespace seop::msg
{
class Message;
}

namespace seop::primive
{
class Vertex_pcs;
}

namespace seop::graphic
{
class Shader_program;

enum Compute_type : uint32_t {
    None = 0,
    Gravity = 1 << 1,
    Electromagnetic = 1 << 2,
    Time_varying_EM_field = 1 << 3,

    Lorenz_equation = 1 << 4,
    End = 1 << 5,
};

class Device_data
{
  public:
    float        fade_scale{0.1f};
    float        frame_rate{240.0f};
    float        inv_frame_rate{1 / frame_rate};
    math::Vec4   back_col = {0.0f, 0.0f, 0.0f, 1.0f};
    Compute_type compute_type{Compute_type::Time_varying_EM_field};
};

enum class Program_type {

    Compute_arrow_field,
    Compute_static_magnetic_field,
    Compute_lorenz_equation,

    Render_arrow,
    Render_particle,
    Render_screen_quad,
    Render_grid,
    Render_wire, // test
    End,
};

enum class Frame_buffer_type {
    Particle_layer,
    End,
};
constexpr size_t FRAME_BUFFER_MAX = static_cast<size_t>(Frame_buffer_type::End);
constexpr size_t SHADER_TASK_MAX = static_cast<size_t>(Program_type::End);

class Device final
{
  public:
    Device();
    ~Device() noexcept = default;

    void init(Context& ctx);
    void update(Context& ctx);
    void clear_frame();
    void shut_down();
    void compute(Context& ctx);
    void render(Context& ctx);
    auto on_message(msg::Message& msg) -> bool;

    void update_frame_buffer(Frame_buffer_type type, int width, int height);
    void update_shader_buffer(uint32_t buffer_id, uint32_t bind_slot, GLsizeiptr buffer_size, const void* data);

    [[nodiscard]] auto data() const -> const Device_data&;
    [[nodiscard]] auto data() -> Device_data&;
    void               set_fade_scale(float scale);
    void               set_frame_rate(float rate);
    void               set_back_col(const math::Vec4& col);
    void               set_compute_type(Compute_type type);
    void               set_shader_task(Context& ctx);

    // temp
    enum class Shader_buffer_slot {
        Particle,
        Wire,
        Arrow,
        End,
    };
    bool             view_electric_field_arrow{false};
    bool             view_magnetic_field_arrow{false};
    bool             view_poynting_field_arrow{false};
    int              arrow_range[6]{-1000, 1000, -1000, 1000, -1000, 1000}; // -x x -y y -z z
    int              arrow_interval{200};
    float            arrow_scale{1.0f};
    float            arrow_thickness{1.0f};
    item::Arrow_node arrow_nodes_;
    void             init_arrow();
    void             create_arrow(int interval, int* range);

    void             prepare_arrow(Context& ctx);
    void             draw_arrow();

    void             set_uniform_scene_data(uint32_t program_id, Context& ctx);
    void             set_uniform_time_data(uint32_t program_id, Context& ctx);

  private:
    void prepare_grid(Context& ctx);
    void prepare_screen_quad(Context& ctx, bool use_tex);
    void prepare_particle(Context& ctx);
    void prepare_wire(Context& ctx);
    void draw_grid();
    void draw_screen_quad();
    void draw_particle(Context& ctx);
    void draw_wire(Context& ctx);

    void set_compute_program(Program_type type, const std::string& cs_path,
                             std::function<void(uint32_t, Context&)> uniform_setter);
    void set_render_program(Program_type type, const std::string& vs_path, const std::string& fs_path,
                            std::function<void(uint32_t, Context&)> uniform_setter);
    void set_frame_buffer(Frame_buffer_type type, int width, int height);

    void init_screen_qaud();
    void init_grid();
    void init_particle(Context& ctx);
    void init_wire(Context& ctx);

    void hot_load_shader(Context& ctx);

  private:
    Device_data                                        data_;
    std::array<Shader_program, SHADER_TASK_MAX>        programs_;
    std::array<opengl::Frame_buffer, FRAME_BUFFER_MAX> frame_buffers_;
    item::Screen_quad                                  screen_quad_;
    item::Grid_quad                                    grid_quad_;
};
} // namespace seop::graphic