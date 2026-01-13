#pragma once

#include <memory>

namespace seop
{
class Context;
}

namespace seop::imgui
{

class Imgui_state
{
  public:
    bool is_ui_hovered{false};
    bool is_reset{false};
};

class Imgui_renderer
{
  public:
    Imgui_renderer() noexcept;
    ~Imgui_renderer() = default;

    void init         (Context &ctx);
    void update       (Context &ctx);
    void end_frame    ();
    void shut_down    ();

    [[nodiscard]] auto state() const -> const Imgui_state &;
    [[nodiscard]] auto inv_frame_rate() const -> float;

  private:
    void show_frame_rate              ();
    void show_particle_properties     (Context &ctx);
    void show_camera_properties       (Context &ctx);
    void show_device_Compute_type     (Context &ctx);
    void show_scene_force             (Context &ctx);

  private:
    Imgui_state state_;
    float frame_rate_       {60.0f};
    float inv_frame_rate_   {1 / 60.0f};
};

inline constexpr float MIN_GRAITY             = 0.0f;
inline constexpr float MAX_GRAITY             = 1e6f;
inline constexpr float MIN_VORTEX             = 0.0f;
inline constexpr float MAX_VORTEX             = 1e2f;
inline constexpr float MIN_DAMPING            = 0.9f;
inline constexpr float MAX_DAMPING            = 1.0f;
inline constexpr float MIN_MAGNETIC_STRENGTH  = 0.0f;
inline constexpr float MAX_MAGNETIC_STRENGTH  = 100.0f;
inline constexpr float MIN_CURRENT_INTENSITY  = -1000.0f;
inline constexpr float MAX_CURRENT_INTENSITY  = 1000.0f;
inline constexpr float MIN_CAMERA_SPEED       = 100.0f;
inline constexpr float MAX_CAMERA_SPEED       = 3000.0f;
} // namespace seop::imgui