#pragma once
#include "camera.hpp"
#include "seop_context/context.hpp"
#include <vector>

namespace seop
{
class Context;
}

namespace seop::entity
{
class Particle;
class Attractor;
} // namespace seop::entity

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
    size_t count{50000};
    // temp
    float  time_scale{1.0f};
};

class Attractor_data
{
  public:
    size_t attractor_count{0};
};

class Scene_electronic_properites
{
  public:
    float I{0.0f};
};

class Scene_magnetic_properites
{
  public:
    float      I{0.0f};
};

class Scene_entities
{
  public:
    std::vector<entity::Attractor> attractors;
    std::vector<entity::Particle>  particles;
};

class Scene_context
{
  public:
};

class Scene_data

{
  public:
    Scene_force                 forces;
    Scene_entities              entities;
    Particle_propetires         particle_properties;
    Attractor_data              attractor_properties;
    Scene_electronic_properites electronic_properites;
    Scene_magnetic_properites   magnetic_properites;
};

class Scene

{
  public:
    Scene() noexcept;
    ~Scene() = default;
    void               init();
    void               update();
    void               end_frame();

    void               reset();
    void               register_commnad(Context& ctx);
    void               create_particles(size_t count);
    void               create_attractors(size_t count);
    void               update_camera(Context& ctx);
    auto               get_col(float t) -> math::Vec4&;

    [[nodiscard]] auto data() const -> const Scene_data&;
    [[nodiscard]] auto data() -> Scene_data&;
    [[nodiscard]] auto camera() const -> const Camera&;
    [[nodiscard]] auto camera() -> Camera&;

    // temp

    auto               get_hash(uint32_t v) -> uint32_t;
    auto               int_to_float(int v) -> float;

  private:
    void                    create_col_table();
    std::vector<math::Vec4> col_table;
    Scene_data              data_;
    Camera                  camera_;
};

} // namespace seop::scene