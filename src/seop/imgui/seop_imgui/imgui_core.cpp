#include "imgui_core.hpp"
#include "seop_context/context.hpp"
#include "seop_window/glf_window.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace seop::imgui
{
Imgui_core::Imgui_core() noexcept : renderer_(std::make_unique<Imgui_renderer>())
{
}

void Imgui_core::init(Context& ctx)
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    // 백엔드 연결
    ImGui_ImplGlfw_InitForOpenGL(ctx.window->glfw_window(), true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void Imgui_core::update()
{
    state_.is_ui_hovered = renderer_->hovering_ui();
}

void Imgui_core::render(Context& ctx)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderer_->begin();
    renderer_->show_frame_rate(ctx);
    renderer_->show_particle_properties(ctx);
    renderer_->show_camera_properties(ctx);
    renderer_->show_device_data(ctx);
    renderer_->show_scene_force(ctx);
    renderer_->end();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Imgui_core::end_frame()
{
    state_.is_reset = false;
    state_.is_ui_hovered = false;
}

void Imgui_core::shut_down()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

auto Imgui_core::state() const -> const Imgui_state&
{
    return state_;
}

} // namespace seop::imgui