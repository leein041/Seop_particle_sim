#pragma once
#include "imgui_dockspace.hpp"
#include "imgui_renderer.hpp"

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

class Imgui_core
{
  public:
    Imgui_core() noexcept;
    ~Imgui_core() = default;

    void               init(Context& ctx);
    void               update();
    void               render(Context& ctx);
    void               end_frame();
    void               shut_down();

    [[nodiscard]] auto state() const -> const Imgui_state&;

  private:
    Imgui_renderer renderer_;
    Dockspace      dockspace_;
    Imgui_state    state_;
};

} // namespace seop::imgui