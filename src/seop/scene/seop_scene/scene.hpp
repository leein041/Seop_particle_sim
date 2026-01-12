#pragma once
#include "camera.hpp"
#include "seop_entity/attractor.hpp"
#include "seop_entity/particle.hpp"
#include "seop_input/input.hpp"
#include "seop_window/glf_window.hpp"
#include "seop_math/math.hpp"
#include <vector>

namespace seop::scene
{
enum Force_type : uint32_t
{
    None = 0,
    Gravity = 1 << 0,
    Vortex = 1 << 1,
    Magnetic = 1 << 2, // z+ dirtection.
    End = 1 << 3,
};

struct Scene_force
{
    uint32_t type{0};
    float gravity{0.0f};
    float vortex{0.0f};
    float damping{0.98f}; // FIX : weird naming
    float magentic_str{0.0f};
};

class Particle_data
{
  public:
    bool particle_reset{false};
    float particle_size{1.0f};
    float particle_col{0.0f};
    size_t particle_count{50000};
};

class Attractor_data
{
  public:
    size_t attractor_count{0};
};

class Scene_electronical_properites
{
  public:
    float I{0.0f};
};

class Scene_entities
{
  public:
    std::vector<entity::Attractor> attractors;
    std::vector<entity::Particle> particles;
};

class Scene_context
{
  public:
    bool is_reset{false};

    bool left_pressed{false};
    const math::Vec2 &cursor_pos;
    const math::Vec2 &half_window;
};

class Scene_data
{
  public:
    Scene_force forces;
    Scene_entities entities;
    Particle_data particle_properties;
    Attractor_data attractor_properties;
    Scene_electronical_properites electronical_properites;
};

class Scene

{
  public:
    Scene() noexcept;
    ~Scene() = default;
    void init();
    void update();

    void create_particles(size_t count);
    void create_attractors(size_t count);

    void update_camera(bool ui_hovered, float dt, input::Input &input, window::Glf_window &window);

    [[nodiscard]] auto data() const -> const Scene_data &;
    [[nodiscard]] auto data() -> Scene_data &;
    [[nodiscard]] auto camera() const -> const Camera &;
    [[nodiscard]] auto camera() -> Camera &;

    // temp
    auto get_col(float t) -> math::Vec4 &;
    std::vector<math::Vec4> col_table;

  private:
    void create_col_table();
    Scene_data data_;
    Camera camera_;
};

} // namespace seop::scene