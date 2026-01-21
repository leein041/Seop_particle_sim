#pragma once
#include "seop_item/item.hpp"
#include "seop_primitive/vertex.hpp"

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

namespace seop::primive
{
class Vertex_pcs;
}

namespace seop::raytrace
{
class Ray;
}

namespace seop::graphic
{
enum Compute_type : uint32_t {
    None = 0,
    Gravity = 1 << 1,
    Electromagnetic = 1 << 2,
    Time_varying_EM_field = 1 << 3,
    End = 1 << 4,
};

class Device_data
{
  public:
    float        fade_scale{0.1f};
    float        frame_rate{60.0f};
    float        inv_frame_rate{1 / 60.0f};
    math::Vec4   back_col = {0.0f, 0.0f, 0.0f, 1.0f};
    Compute_type compute_type{Compute_type::Time_varying_EM_field};
};

// new
using Uniform_setter = std::function<void(Context&)>;
enum class Shader_task_type {
    Compute_gravity,
    Compute_electromagnetic,
    Compute_static_magnetic_field,

    Render_particle,
    Render_screen_quad,
    Render_grid,
    Render_wire, // test
    End,
};
enum class Shader_buffer_type {
    Particle,
    Conductor_wire,
    End,
};
enum class Frame_buffer_type {
    Particle_layer,
    End,
};
constexpr size_t FRAME_BUFFER_MAX = static_cast<size_t>(Frame_buffer_type::End);
constexpr size_t SHADER_TASK_MAX = static_cast<size_t>(Shader_task_type::End);
constexpr size_t SHADER_BUFFER_MAX = static_cast<size_t>(Shader_buffer_type::End);

class Shader_task
{
  public:
    uint32_t       program_id{0};
    Uniform_setter uniform_setter;
};

class Shader_buffer
{
  public:
    uint32_t id;
};

class Frame_buffer
{
  public:
    bool     use_tex{false};
    uint32_t buf_id;
    uint32_t tex_id;
};
// temp
enum class Edit_mode { Plane_xy, Plane_yz, Plane_zx, End };

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
    void update_frame_buffer(Frame_buffer_type type, int width, int height);
    void update_shader_buffer(uint32_t buffer_id, uint32_t binding_point, GLsizeiptr buffer_size, const void* data);
    void update_vertex_buffer(uint32_t buffer_id, GLsizeiptr buffer_size, const void* data);

    [[nodiscard]] auto data() const -> const Device_data&;
    [[nodiscard]] auto data() -> Device_data&;
    void               set_fade_scale(float scale);
    void               set_frame_rate(float rate);
    void               set_back_col(const math::Vec4& col);
    void               set_compute_type(Compute_type type);

    // TODO : temp region need refactoring
    // temp
    enum Vertex_state : uint16_t {
        STATE_NONE = 0,
        STATE_HOVER_ONLY = 1 << 1,
        STATE_HOVER_WIRE = 1 << 2,
        STATE_SELECTED = 1 << 3,
        STATE_HOLD = 1 << 4,
        STATE_INACTIVE = 1 << 5,
        STATE_DESTROYED = 1 << 6,
    };

    float                snap_interval{50.0f};
    math::Closest_points closest_points_to_line;
    Edit_mode            edit_mode_{Edit_mode::Plane_xy};

    math::Vec3           pre_snapped_pos{math::Vec3::Zero};
    math::Vec3           cur_intersection{math::Vec3::Zero};
    item::Wire           test_wires_;

    void                 create_wire();
    void                 add_wire();
    auto                 make_ray(const math::Vec2& ndc_pos, const math::Vec3& cam_pos, const math::Matrix& view,
                                  const math::Matrix& projection) -> raytrace::Ray;
    void                 raytrace(Context& ctx);
    auto get_snap_pos(float interval, const math::Vec3& ray_ori, const math::Vec3& ray_dir, const math::Vec3& vert_pos,
                      Edit_mode mode) -> math::Vec3;
    // temp end
  private:
    void     add_shader_buffer(Shader_buffer_type type);
    void     add_compute_task(Shader_task_type type, const char* cs_src,
                              std::function<void(uint32_t, Context&)> uniform_setter);
    void     add_render_task(Shader_task_type type, const char* vs_src, const char* fs_src,
                             std::function<void(uint32_t, Context&)> uniform_setter);
    void     add_frame_buffer(Frame_buffer_type type, int width, int height);

    void     create_screen_qaud();
    void     create_grid();
    uint32_t create_cs(const char* src);
    uint32_t create_vs(const char* src);
    uint32_t create_fs(const char* src);

    uint32_t particle_layer_buffer;
    uint32_t particle_layer_tex;

  private:
    Device_data                                  data_;
    std::array<Shader_buffer, SHADER_BUFFER_MAX> shader_buffers_;
    std::array<Shader_task, SHADER_TASK_MAX>     shader_tasks_;
    std::array<Frame_buffer, FRAME_BUFFER_MAX>   frame_buffers_;

    item::Render_item<primitive::Vertex_pu>      screen_quad_;
    item::Render_item<primitive::Vertex_pc>      grid_quad_;
};
} // namespace seop::graphic