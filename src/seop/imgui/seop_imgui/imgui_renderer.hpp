#pragma once

namespace seop
{
class Context;
}

namespace seop::imgui
{

class Imgui_renderer
{
  public:
    Imgui_renderer() noexcept;
    ~Imgui_renderer() = default;

    void               init();
    void               begin();
    void               end();

    void               show_frame_rate(Context& ctx);
    void               show_particle_properties(Context& ctx);
    void               show_camera_properties(Context& ctx);
    void               show_device_data(Context& ctx);
    void               show_scene_data(Context& ctx);

    [[nodiscard]] auto hovering_ui() -> bool;

  private:
};

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
} // namespace seop::imgui