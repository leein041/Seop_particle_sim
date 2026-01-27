#pragma once
#include "camera.hpp"
#include "seop_context/context.hpp"
#include "seop_item/item.hpp"
#include "seop_item/wire.hpp"

#include <vector>

namespace seop
{
class Context;
}

namespace seop::scene
{
enum Force_type : uint32_t {
    None = 0,
    Gravity = 1 << 0,
    Vortex = 1 << 1,
    Magnetic = 1 << 2, // z+ dirtection.
    End = 1 << 3,
};

struct Scene_force
{
    uint32_t type{0};
    float    gravity{0.0f};
    float    vortex{0.0f};
    float    damping{0.98f}; // FIX : weird naming
    float    magentic_str{0.0f};
};

class Particle_propetires
{
  public:
    bool   particle_reset{false};
    float  size{1.0f};
    float  col{0.0f};
    size_t count{0};
    // temp
    float  time_scale{1.0f};
};

class Attractor_data
{
  public:
    size_t attractor_count{0};
};

class Wire_properites
{
  public:
    float           wire_current{0.0f};
    float           wire_current_dt{0.0f};
    float           w = 2.0f; // 각 주파수
    float           max_i = 10.0f;
    float           c = 2000.0f; // 광속
    item::Wire_node wire_nodes;
};

class Scene_entities
{
  public:
    item::Particle particles;
};

class Scene_context
{
  public:
};

class Scene_data

{
  public:
    float               scene_speed{1.0f};
    Scene_force         forces;
    Scene_entities      entities;
    Particle_propetires particle_properties;
    Attractor_data      attractor_properties;
    Wire_properites     wire_properites;
};

class Scene

{
  public:
    Scene() noexcept;
    ~Scene() = default;
    void               init();
    void               update(Context& ctx);
    void               end_frame();

    void               reset();
    void               register_commnad(Context& ctx);
    void               create_particles_sphere_group(size_t count);
    void               create_particles_cube_group(size_t count);
    void               add_wire();

    [[nodiscard]] auto data() const -> const Scene_data&;
    [[nodiscard]] auto data() -> Scene_data&;
    [[nodiscard]] auto camera() const -> const Camera&;
    [[nodiscard]] auto camera() -> Camera&;

    // temp
    auto               get_hash(uint32_t v) -> uint32_t;
    auto               int_to_float(int v) -> float;

  private:
    Scene_data data_;
    Camera     camera_;
};

} // namespace seop::scene