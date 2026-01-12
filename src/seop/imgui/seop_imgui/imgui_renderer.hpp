#pragma once

#include "seop_entity/particle.hpp"
#include "seop_scene/scene.hpp"
#include "seop_window/glf_window.hpp"

#include <memory>

namespace seop::imgui
{
inline constexpr float MIN_GRAITY = 0.0f;
inline constexpr float MAX_GRAITY = 1e6f;

inline constexpr float MIN_VORTEX = 0.0f;
inline constexpr float MAX_VORTEX = 1e2f;

inline constexpr float MIN_DAMPING = 0.9f;
inline constexpr float MAX_DAMPING = 1.0f;

inline constexpr float MIN_MAGNETIC_STRENGTH = 0.0f;
inline constexpr float MAX_MAGNETIC_STRENGTH = 100.0f;

inline constexpr float MIN_CURRENT_INTENSITY = -1000.0f;
inline constexpr float MAX_CURRENT_INTENSITY = 1000.0f;

inline constexpr float MIN_CAMERA_SPEED = 100.0f;
inline constexpr float MAX_CAMERA_SPEED = 3000.0f;

class Imgui_context
{
  public:
    bool is_ui_hovered{false};
};

class Imgui_renderer
{
  public:
    Imgui_renderer() noexcept;
    ~Imgui_renderer() = default;

    void init(const window::Glf_window &window);
    void render(scene::Scene_data &scene_data, scene::Camera & cam);
    void end_frame();
    void shut_down();

    [[nodiscard]] auto inv_frame_rate() const -> float;
    [[nodiscard]] auto is_hovered() const -> bool;

    // temp

    [[nodiscard]] auto is_secene_reset() -> bool;
    bool scene_reset_{false};

  private:
    void show_frame_rate();
    void show_scene_force(scene::Scene_force &forces);
    void show_particle_properties(scene::Particle_data &p);
    void show_electronical_properties(scene::Scene_electronical_properites &p);
    void show_camera_properties(scene::Camera &c);

    float frame_rate_{60.0f};
    float inv_frame_rate_{1 / 60.0f};
};
} // namespace seop::imgui