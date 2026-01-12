#include "imgui_renderer.hpp"
#include "seop_scene/scene.hpp"
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

void Imgui_renderer::init(const window::Glf_window &window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // 스타일 설정
    ImGui::StyleColorsDark();

    // 백엔드 연결
    ImGui_ImplGlfw_InitForOpenGL(window.glfw_window(), true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void Imgui_renderer::render(scene::Scene_data &scene_data, scene::Camera &cam)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowBgAlpha(0.25f); // 배경 투명도
    ImGui::Begin("Debug Panel");        // 제목

    show_frame_rate();
    show_scene_force(scene_data.forces);
    show_particle_properties(scene_data.particle_properties);
    show_camera_properties(cam);

    ImGui::End(); // 창 종료

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Imgui_renderer::end_frame()
{
}

void Imgui_renderer::shut_down()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

auto Imgui_renderer::inv_frame_rate() const -> float
{
    return inv_frame_rate_;
}

auto Imgui_renderer::is_hovered() const -> bool
{
    return ImGui::GetIO().WantCaptureMouse;
}

auto Imgui_renderer::is_secene_reset() -> bool
{
    return scene_reset_;
}

void Imgui_renderer::show_frame_rate()
{
    if (ImGui::SliderFloat("Frame Rate", &frame_rate_, 1.0f, 240.0f, "%1.0f")) {
        inv_frame_rate_ = 1 / frame_rate_;
    }

    ImDrawList *draw_list = ImGui::GetBackgroundDrawList();

    std::string text = std::format("FPS: {:.1f}", ImGui::GetIO().Framerate);
    float x = 10.0f;
    float y = 10.0f;
    draw_list->AddText(ImVec2(x, y), IM_COL32(255, 255, 255, 255), text.c_str());
    draw_list->AddText(ImVec2(x + 1, y + 1), IM_COL32(0, 0, 0, 255), text.c_str());
}

void Imgui_renderer::show_scene_force(scene::Scene_force &forces)
{

    ImGui::SliderFloat("Gravity", &forces.gravity, MIN_GRAITY, MAX_GRAITY, "%1.f");
    ImGui::SliderFloat("Vortex", &forces.vortex, MIN_VORTEX, MAX_VORTEX, "%.2f");
    ImGui::SliderFloat("Damping", &forces.damping, MIN_DAMPING, MAX_DAMPING, "%.3f");
    ImGui::SliderFloat("Magnetic Strength", &forces.magentic_str, MIN_MAGNETIC_STRENGTH, MAX_MAGNETIC_STRENGTH, "%.3f");

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

void Imgui_renderer::show_particle_properties(scene::Particle_data &p)
{
    ImGui::SliderFloat("Particle size", &p.particle_size, 1.0f, 10.0f, "%1.0f");
    ImGui::SliderFloat("Particle color", &p.particle_col, 0.0f, 1.0f, "%.3f");

    p.particle_reset = false;
    if (ImGui::Button("Reset")) {
        p.particle_reset = true;
    }
#if 1
    size_t cnt = p.particle_count;
    ImGui::InputScalar("Particle count", ImGuiDataType_U64, &cnt, NULL, NULL, "%zu");

    if (ImGui::IsItemDeactivatedAfterEdit()) // enter
    {
        p.particle_count = cnt;
        p.particle_reset = true;
    }
#endif
}

void Imgui_renderer::show_electronical_properties(scene::Scene_electronical_properites &p)
{
    ImGui::SliderFloat("Current Intensity", &p.I, MIN_CURRENT_INTENSITY, MAX_CURRENT_INTENSITY, "%.3f");
}

void Imgui_renderer::show_camera_properties(scene::Camera &c)
{
    ImGui::DragFloat3("Camera Pos", &c.data().transform.pos.x_);
    ImGui::SliderFloat("Camera Speed", &c.data().transform.speed_scale, MIN_CAMERA_SPEED, MAX_CAMERA_SPEED, "%.2f");
}
} // namespace seop::imgui