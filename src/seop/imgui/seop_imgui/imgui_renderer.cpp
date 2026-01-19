#include "imgui_renderer.hpp"

#include "seop_context/context.hpp"
#include "seop_graphic/device.hpp"
#include "seop_item/item.hpp"
#include "seop_primitive/vertex.hpp"
#include "seop_scene/scene.hpp"
#include "seop_scene/scene_command.hpp"
#include "seop_scene/scene_message.hpp"

#include <imgui/imgui.h>

#include <format>
#include <iostream>

namespace seop::imgui
{

Imgui_renderer::Imgui_renderer() noexcept
{
}

void Imgui_renderer::show_frame_rate(Context& ctx)
{
    ImGui::SetNextWindowBgAlpha(0.5f); // 배경 투명도
    ImGui::Begin("frame setting");     // 제목

    graphic::Device_data& data = ctx.device->data();
    if (ImGui::SliderFloat("Frame Rate", &data.frame_rate, 1.0f, 240.0f, "%1.0f")) {
        data.inv_frame_rate = 1 / data.frame_rate;
    }

    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

    std::string text = std::format("FPS: {:.1f}", ImGui::GetIO().Framerate);
    float       x = 10.0f;
    float       y = 20.0f;
    draw_list->AddText(ImVec2(x, y), IM_COL32(255, 255, 255, 255), text.c_str());
    draw_list->AddText(ImVec2(x + 1, y + 1), IM_COL32(0, 0, 0, 255), text.c_str());

    ImGui::End();
}

void Imgui_renderer::show_scene_data(Context& ctx)
{
    ImGui::SetNextWindowBgAlpha(0.25f);
    ImGui::Begin("Scene setting");

    scene::Scene_data& data = ctx.scene->data();

    if (ctx.device->data().compute_type == graphic::Compute_type::Gravity) {
        ImGui::SliderFloat("Gravity Intensity", &data.forces.gravity, MIN_GRAITY, MAX_GRAITY, "%1.f");
        ImGui::SliderFloat("Vortex", &data.forces.vortex, MIN_VORTEX, MAX_VORTEX, "%.2f");
        ImGui::SliderFloat("Damping", &data.forces.damping, MIN_DAMPING, MAX_DAMPING, "%.3f");
    } else if (ctx.device->data().compute_type == graphic::Compute_type::Electromagnetic) {
        ImGui::SliderFloat("Magnetic Strength", &data.forces.magentic_str, MIN_MAGNETIC_STRENGTH, MAX_MAGNETIC_STRENGTH,
                           "%.3f");
    } else if (ctx.device->data().compute_type == graphic::Compute_type::Magnetic) {
    
    }

    ImGui::End();
}

auto Imgui_renderer::hovering_ui() -> bool
{
    return ImGui::GetIO().WantCaptureMouse;
}

void Imgui_renderer::show_particle_properties(Context& ctx)
{
    ImGui::SetNextWindowBgAlpha(0.25f);
    ImGui::Begin("Particle Setting");

    scene::Particle_propetires& properties = ctx.scene->data().particle_properties;
    ImGui::SliderFloat("Particle time", &properties.time_scale, -5.0f, 10.0f, "%.1f");
    ImGui::SliderFloat("Particle size", &properties.size, 1.0f, 10.0f, "%1.0f");
    ImGui::SliderFloat("Particle color", &properties.col, 0.0f, 1.0f, "%.3f");

#if 1
    size_t cnt = properties.count;
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

    ImGui::End();
}

void Imgui_renderer::show_camera_properties(Context& ctx)
{
    ImGui::SetNextWindowBgAlpha(0.25f);
    ImGui::Begin("Camera setting");

    scene::Camera_data& cam = ctx.scene->camera().data();
    ImGui::DragFloat3("Camera Pos", &cam.transform.pos.x_);
    ImGui::SliderFloat("Camera Speed", &cam.transform.speed_scale, MIN_CAMERA_SPEED, MAX_CAMERA_SPEED, "%.2f");

    ImGui::End();
}

void Imgui_renderer::show_device_data(Context& ctx)
{
    ImGui::SetNextWindowBgAlpha(0.25f);
    ImGui::Begin("Device setting");

    if (ImGui::Button("Gravity")) {
        ctx.device->set_compute_type(graphic::Compute_type::Gravity);
    }
    if (ImGui::Button("Electromagnetic")) {
        ctx.device->set_compute_type(graphic::Compute_type::Electromagnetic);
    }
    if (ImGui::Button("Magnetic Field")) {
        ctx.device->set_compute_type(graphic::Compute_type::Magnetic);
    }
    ImGui::SliderFloat("fade speed", &ctx.device->data().fade_scale, 1.0f, 0.001f, "%.3f");

    ImGui::End();
}

void Imgui_renderer::init()
{
    IMGUI_CHECKVERSION();
}

void Imgui_renderer::begin()
{

    ImGui::SetNextWindowBgAlpha(0.25f); // 배경 투명도
    ImGui::Begin("Debug Panel");        // 제목
}

void Imgui_renderer::end()
{
    ImGui::End(); // 창 종료
}
} // namespace seop::imgui
