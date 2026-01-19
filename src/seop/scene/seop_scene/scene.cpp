#include "scene.hpp"
#include "scene_command.hpp"
#include "seop_entity/attractor.hpp"
#include "seop_entity/particle.hpp"
#include "seop_input/input.hpp"
#include "seop_math/math.hpp"
#include "seop_window/glf_window.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace seop::scene
{
using namespace math;

Scene::Scene() noexcept
{
}

void Scene::init()
{
    camera_.init();
    create_col_table();
    create_particles(data_.particle_properties.count);
    create_attractors(data_.attractor_properties.attractor_count);
}

void Scene::update()
{
}

void Scene::end_frame()
{
    camera_.reset_state();
}

void Scene::reset()
{
    Scene_data new_data;
    data_ = new_data;
    create_particles(data_.particle_properties.count);
    create_attractors(data_.attractor_properties.attractor_count);
}

void Scene::register_commnad(Context& ctx)
{
    ctx.command_list->register_command<Particle_change_command>(data_.particle_properties.count, *this);
}

auto Scene::camera() const -> const Camera&
{
    return camera_;
}

auto Scene::camera() -> Camera&
{
    return camera_;
}

auto Scene::get_hash(uint32_t v) -> uint32_t
{

    uint32_t state = v * 747796405u + 2891336453u;
    uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

void Scene::create_col_table()
{
    for (int i = 0; i < 256; ++i) {
        float t = static_cast<float>(i) / 255.0f;

        // 파랑(0.0)에서 빨강(1.0)으로 보간
        col_table.push_back(math::Vec4{t, 0.0f, 1.0f - t, 1.0f});
    }
}

auto Scene::int_to_float(int v) -> float
{
    return static_cast<float>(v) * (1.0f / 4294967296.0f);
}

auto Scene::get_col(float t) -> math::Vec4&
{
    int idx = static_cast<int>(t * 255.0f);
    idx = std::clamp(idx, 0, 255);
    return col_table[idx];
}

void Scene::create_particles(size_t count)
{
    data_.particle_properties.count = count;
    data_.entities.particles.clear();
    data_.entities.particles.reserve(count);
    for (int i = 0; i < count; ++i) {
        uint32_t   r1_uint = get_hash(i);
        uint32_t   r2_uint = get_hash(i + static_cast<int>(count)); // 오프셋을 주어 겹침 방지
        uint32_t   r3_uint = get_hash(i + static_cast<int>(count) * 2u);

        float      r1 = int_to_float(r1_uint);
        float      r2 = int_to_float(r2_uint);
        float      r3 = int_to_float(r3_uint);

        float      cos_θ = cos(r1 * math::π);
        float      sin_θ = sin(r1 * math::π);
        float      cos_φ = cos(r2 * 2.0f * math::π); // -1 ~ 1
        float      sin_φ = sin(r2 * 2.0f * math::π); // -1 ~ 1
        float      r = r3 * 1000.0f;

        math::Vec4 pos{r * sin_θ * cos_φ, r * sin_θ * sin_φ, r * cos_θ, 1.0f};
        math::Vec4 vel{0.0f, 0.0f, 0.0f, 0.0f};
        math::Vec4 col{1.0f, 1.0f, 1.0f, 1.0f};

        data_.entities.particles.push_back(entity::Particle{pos, vel, col});
    }
}

void Scene::create_attractors(size_t count)
{
    data_.entities.attractors.clear();
    data_.entities.attractors.reserve(count);
    for (int i = 0; i < count; ++i) {
        data_.entities.attractors.push_back(
            entity::Attractor{math::Vec4{200.0f, 200.0f, 200.0f, 1.0f}, 2000.0f, 1000.0f});
    }
}

void Scene::update_camera(Context& ctx)
{
    Camera_state& cam_state = camera_.camera_state();
    Camera_trasform& tr = camera_.data().transform;
    // NOTE : it's RH
    if (ctx.input->get_key(input::Key_code::W)) {
        tr.pos += tr.forward * tr.speed_scale * ctx.f_dt;
        cam_state.is_move = true;
    }
    if (ctx.input->get_key(input::Key_code::S)) {
        tr.pos -= tr.forward * tr.speed_scale * ctx.f_dt;
        cam_state.is_move = true;
    }
    if (ctx.input->get_key(input::Key_code::A)) {
        tr.pos -= tr.right * tr.speed_scale * ctx.f_dt;
        cam_state.is_move = true;
    }
    if (ctx.input->get_key(input::Key_code::D)) {
        tr.pos += tr.right * tr.speed_scale * ctx.f_dt;
        cam_state.is_move = true;
    }
    if (ctx.input->get_key(input::Key_code::E)) {
        tr.pos += tr.up * tr.speed_scale * ctx.f_dt;
        cam_state.is_move = true;
    }
    if (ctx.input->get_key(input::Key_code::Q)) {
        tr.pos -= tr.up * tr.speed_scale * ctx.f_dt;
        cam_state.is_move = true;
    }

    if (ctx.input->get_key(input::Key_code::RButton)) {
        Vec2  delta = ctx.input->get_delta_cursor_pos();

        float dx = delta.x_ * ctx.input->mouse_sensitivity;
        float dy = delta.y_ * ctx.input->mouse_sensitivity;

        // 각도 업데이트
        tr.yaw += dx;
        tr.pitch -= dy;
        cam_state.is_move = true;

    }
    if (ctx.input->get_key_down(input::Key_code::LButton)) {
        Vec2   pos_ndc = ctx.window->get_cursor_pos_ndc();
        Vec4   clip_space(pos_ndc.x_, pos_ndc.y_, -1.0f, 1.0f);
        Matrix projection_matirx_inv = camera_.data().projection.Invert();
        Matrix view_matirx_inv = camera_.data().view.Invert();

        Vec4   view_space = projection_matirx_inv * clip_space;
        view_space.z_ = -1.0f;
        view_space.w_ = 0.0f;
        Vec4 ray_direction = view_matirx_inv * view_space;
        Vec3 eff_ray_dir = ray_direction.xyz();
        eff_ray_dir.Normalize();
        float ray_distance = 300.0f;
        ctx.input->set_ray_point(eff_ray_dir * ray_distance + tr.pos);
    }
    // 위치가 변했으므로 행렬 다시 계산
    // test
    camera_.update();
}

auto Scene::data() const -> const Scene_data&
{
    return data_;
}

auto Scene::data() -> Scene_data&
{
    return data_;
}

} // namespace seop::scene