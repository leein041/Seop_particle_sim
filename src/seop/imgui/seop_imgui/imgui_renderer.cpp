#include "imgui_renderer.hpp"
#include "seop_context/context.hpp"

#include "seop_scene/scene.hpp"
#include "seop_scene/scene_command.hpp"
#include "seop_scene/scene_message.hpp"
#include "seop_window/glf_window.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <format>
#include <imgui/imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

namespace seop::imgui
{
Imgui_renderer::Imgui_renderer() noexcept
{
}

void Imgui_renderer::init(Context& ctx)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // 스타일 설정
    ImGui::StyleColorsDark();

    // 백엔드 연결
    ImGui_ImplGlfw_InitForOpenGL(ctx.window->glfw_window(), true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void Imgui_renderer::update(Context& ctx)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowBgAlpha(0.25f); // 배경 투명도
    ImGui::Begin("Debug Panel");        // 제목

    state_.is_ui_hovered = ImGui::GetIO().WantCaptureMouse;
    if (ImGui::Button("Reset")) {
        state_.is_reset = true;
    }

    show_frame_rate();
    show_scene_force(ctx);
    show_particle_properties(ctx);
    show_camera_properties(ctx);

    ImGui::End(); // 창 종료
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Imgui_renderer::end_frame()
{
    // reset context
    state_.is_reset = false;
    state_.is_ui_hovered = false;
}

void Imgui_renderer::shut_down()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

auto Imgui_renderer::state() const -> const Imgui_state&
{
    return state_;
}

auto Imgui_renderer::inv_frame_rate() const -> float
{
    return inv_frame_rate_;
}

void Imgui_renderer::show_frame_rate()
{
    if (ImGui::SliderFloat("Frame Rate", &frame_rate_, 1.0f, 240.0f, "%1.0f")) {
        inv_frame_rate_ = 1 / frame_rate_;
    }

    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

    std::string text = std::format("FPS: {:.1f}", ImGui::GetIO().Framerate);
    float       x = 10.0f;
    float       y = 10.0f;
    draw_list->AddText(ImVec2(x, y), IM_COL32(255, 255, 255, 255), text.c_str());
    draw_list->AddText(ImVec2(x + 1, y + 1), IM_COL32(0, 0, 0, 255), text.c_str());
}

void Imgui_renderer::show_scene_force(Context& ctx)
{

    ImGui::SliderFloat("Gravity", &ctx.scene->data().forces.gravity, MIN_GRAITY, MAX_GRAITY, "%1.f");
    ImGui::SliderFloat("Vortex", &ctx.scene->data().forces.vortex, MIN_VORTEX, MAX_VORTEX, "%.2f");
    ImGui::SliderFloat("Damping", &ctx.scene->data().forces.damping, MIN_DAMPING, MAX_DAMPING, "%.3f");
    ImGui::SliderFloat("Magnetic Strength", &ctx.scene->data().forces.magentic_str, MIN_MAGNETIC_STRENGTH,
                       MAX_MAGNETIC_STRENGTH, "%.3f");

#if 0
        static bool on_gravity{false};
        if (ImGui::Checkbox("Gravity on/off", &on_gravity))
        {
            if (on_gravity)
            {
                forces.type |= scene::Force_type::Gravity;
            }
            else
            {
                forces.type &= ~scene::Force_type::Gravity;
            }
        }
        static bool on_vortex{false};
        if (ImGui::Checkbox("Vortex on/off", &on_vortex))
        {
            if (on_vortex)
            {
                forces.type |= scene::Force_type::Vortex;
            }
            else
            {
                forces.type &= ~scene::Force_type::Vortex;
            }
        }
#endif
}

void Imgui_renderer::show_particle_properties(Context& ctx)
{
    ImGui::SliderFloat("Particle size", &ctx.scene->data().particle_properties.size, 1.0f, 10.0f, "%1.0f");
    ImGui::SliderFloat("Particle color", &ctx.scene->data().particle_properties.col, 0.0f, 1.0f, "%.3f");

#if 1
    size_t cnt = ctx.scene->data().particle_properties.count;
    ImGui::InputScalar("Particle count", ImGuiDataType_U64, &cnt, NULL, NULL, "%zu");

    if (ImGui::IsItemDeactivatedAfterEdit()) // enter
    {
        if (auto* cmd = ctx.command_list->get<scene::Particle_change_command>()) {
            cmd->set_particle_count(cnt);
            cmd->execute();
            // FIXED : scene 멤버함수내에서 메세지를 발행해야하는데
            // scene 멤버함수가 ms_queue를 알아야함. 이게 싫다면 어케?
            ctx.msg_queue->Push_messag(
                new scene::Particle_change_message(cnt, ctx.scene->data().entities.particles.data()));
        }
    }
#endif
}

void Imgui_renderer::show_electronical_properties(Context& ctx)
{
}

void Imgui_renderer::show_camera_properties(Context& ctx)
{
    ImGui::DragFloat3("Camera Pos", &ctx.scene->camera().data().transform.pos.x_);
    ImGui::SliderFloat("Camera Speed", &ctx.scene->camera().data().transform.speed_scale, MIN_CAMERA_SPEED,
                       MAX_CAMERA_SPEED, "%.2f");
}
} // namespace seop::imgui