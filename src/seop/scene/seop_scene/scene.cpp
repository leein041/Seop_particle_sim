#include "scene.hpp"
#include "scene_command.hpp"
#include "seop_math/math.hpp"
#include "seop_primitive/vertex.hpp"

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
    create_particles_sphere_group(data_.particle_properties.count);
}

void Scene::update(Context& ctx)
{
    data_.wire_properites.wire_current_dt = data_.wire_properites.max_i * data_.wire_properites.w *
                                            cosf(data_.wire_properites.w * static_cast<float>(ctx.d_time));
    data_.wire_properites.wire_current =
        data_.wire_properites.max_i * sinf(data_.wire_properites.w * static_cast<float>(ctx.d_time));
}

void Scene::end_frame()
{
    camera_.reset_state();
}

void Scene::reset()
{
    create_particles_sphere_group(data_.particle_properties.count);
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

auto Scene::int_to_float(int v) -> float
{
    return static_cast<float>(v) * (1.0f / 4294967296.0f);
}

void Scene::create_particles_sphere_group(size_t count)
{
    data_.particle_properties.count = count;
    data_.entities.particles.vb.vertices.clear();
    data_.entities.particles.vb.vertices.reserve(count);
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

        data_.entities.particles.vb.vertices.push_back(primitive::Vertex_pcv{pos, col, vel});
    }
}

void Scene::create_particles_cube_group(size_t count)
{
    data_.particle_properties.count = count;
    data_.entities.particles.vb.vertices.clear();
    data_.entities.particles.vb.vertices.reserve(count);
    for (int i = 0; i < count; ++i) {
        uint32_t   r1_uint = get_hash(i);
        uint32_t   r2_uint = get_hash(i + static_cast<int>(count)); // 오프셋을 주어 겹침 방지
        uint32_t   r3_uint = get_hash(i + static_cast<int>(count) * 2u);

        float      r1 = int_to_float(r1_uint);
        float      r2 = int_to_float(r2_uint);
        float      r3 = int_to_float(r3_uint);

        math::Vec4 pos{r1 * 500.0f, r2 * 500.0f, r3 * 500.0f, 1.0f};
        math::Vec4 vel{0.0f, 0.0f, 0.0f, 0.0f};
        math::Vec4 col{1.0f, 1.0f, 1.0f, 1.0f};

        data_.entities.particles.vb.vertices.push_back(primitive::Vertex_pcv{pos, col, vel});
    }
}

void Scene::add_wire()
{
    data_.wire_properites.wire_nodes.vb.vertices.push_back(primitive::Vertex_pcs{Vec4{0.0f, -100.0f, 0.0f, 1.0f},
                                                            Vec4{1.0f, 1.0f, 0.0f, 1.0f}, 1.0f, 0.0f,
                                                            ++primitive::vert_id, primitive::Vertex_state::STATE_NONE});
    data_.wire_properites.wire_nodes.vb.vertices.push_back(primitive::Vertex_pcs{Vec4{0.0f, 100.0f, 0.0f, 1.0f},
                                                            Vec4{1.0f, 1.0f, 0.0f, 1.0f}, 1.0f, 1.0f,
                                                            ++primitive::vert_id, primitive::Vertex_state::STATE_NONE});

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