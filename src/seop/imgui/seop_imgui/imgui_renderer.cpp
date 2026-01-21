#include "imgui_renderer.hpp"

#include "seop_context/context.hpp"
#include "seop_graphic/device.hpp"
#include "seop_item/item.hpp"
#include "seop_primitive/vertex.hpp"
#include "seop_scene/scene.hpp"
#include "seop_scene/scene_command.hpp"
#include "seop_scene/scene_message.hpp"
#include "seop_window/glf_window.hpp"
#include "seop_window/window_message.hpp"

#include <imgui/imgui.h>

#include <format>
#include <iostream>

namespace seop::imgui
{

Imgui_renderer::Imgui_renderer() noexcept
{
}

void Imgui_renderer::show_menu_bar(Context& ctx)
{
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::BeginMenu("Resolution")) {
                // 해상도 프리셋 정의
                struct Res
                {
                    const char* name;
                    float       w;
                    float       h;
                };

                Res presets[] = {{"1280 x 720", 1280.f, 720.f},
                                 {"1600 x 900", 1600.f, 900.f},
                                 {"1920 x 1080", 1920.f, 1080.f},
                                 {"2560 x 1440", 2560.f, 1440.f}};

                for (auto& r : presets) {
                    if (ImGui::MenuItem(r.name)) {
                        ctx.window->set_window_size(math::Vec2{r.w, r.h});
                        ctx.scene->camera().set_aspect(r.w / r.h);
                        ctx.device->update_frame_buffer(graphic::Frame_buffer_type::Particle_layer,
                                                        static_cast<int>(r.w), static_cast<int>(r.h));
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void Imgui_renderer::show_manual()
{
    ImGui::SetNextWindowBgAlpha(0.25f);
    ImGui::Begin("Manual");
    ImGui::SeparatorText("Shortcut Key");

    auto KeyHelp = [](const char* key, const char* desc) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "[ %s ]", key); // 키를 노란색으로 강조
        ImGui::SameLine(60);                                               // 간격을 맞춰서 정렬
        ImGui::Text("- %s", desc);
    };

    KeyHelp("V", "Create wire");
    KeyHelp("G", "Delete");
    KeyHelp("R", "Invert wire");

    ImGui::End();
}

void Imgui_renderer::show_frame_rate(Context& ctx)
{
    ImGui::SetNextWindowBgAlpha(0.7f);    // 가독성을 위해 살짝 더 불투명하게
    ImGui::Begin("Performance Settings"); // 창 이름 통일

    // 1. FPS 표시 (색상을 넣어 강조)
    float  fps = ImGui::GetIO().Framerate;
    ImVec4 fps_color = (fps > 50.0f) ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.5f, 0.0f, 1.0f);

    ImGui::Text("Application Average: ");
    ImGui::SameLine();
    ImGui::TextColored(fps_color, "%.1f FPS", fps);
    ImGui::Text("Frame Time: %.3f ms/frame", 1000.0f / fps);

    ImGui::Separator(); // 구분선

    // 2. 프레임 레이트 조절 슬라이더
    graphic::Device_data& data = ctx.device->data();
    ImGui::Text("Target Frame Rate");
    if (ImGui::SliderFloat("##fps_slider", &data.frame_rate, 10.0f, 240.0f, "%.0f FPS")) {
        data.inv_frame_rate = 1.0f / data.frame_rate;
    }

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
    } else if (ctx.device->data().compute_type == graphic::Compute_type::Time_varying_EM_field) {
        ImGui::SeparatorText("Current");

        ImGui::Text("Max Current");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN); // 남은 가로 공간을 모두 채움
        ImGui::SliderFloat("##max_current", &data.wire_properites.max_i, -10.0f, 10.0f, "%.1f");

        ImGui::Text("Angle Frequency");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN); // 남은 가로 공간을 모두 채움
        ImGui::SliderFloat("##angle_frequency", &data.wire_properites.w, 1.0f, 100.0f, "%1.f");
    }
    if (ImGui::Button("Add wire")) {
        ctx.device->add_wire();
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
    if (!ImGui::Begin("Particle Option", nullptr)) {
        ImGui::End();
        return;
    }

    scene::Particle_propetires& properties = ctx.scene->data().particle_properties;

    ImGui::SeparatorText("Particle Speed");
    ImGui::Text("Speed");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN); // 남은 가로 공간을 모두 채움
    ImGui::SliderFloat("##particle_time", &properties.time_scale, -5.0f, 10.0f, "%.1f");

    ImGui::SeparatorText("Particle size");
    ImGui::Text("Size");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN); // 남은 가로 공간을 모두 채움
    ImGui::SliderFloat("##particle_size", &properties.size, 1.0f, 10.0f, "%1.0f");

    ImGui::SeparatorText("Particle Color");
    ImGui::Text("Color");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN); // 남은 가로 공간을 모두 채움

    ImVec4 active_col = ImVec4(0.3f, properties.col, 0.7f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, active_col);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, active_col); // 클릭 시 색상

    ImGui::SliderFloat("##particle_color", &properties.col, 0.0f, 1.0f, "%.3f");

    ImGui::PopStyleColor(2);

#if 1
    size_t cnt = properties.count;

    ImGui::SeparatorText("Particle Count");
    ImGui::Text("Input Count");
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.4f); // ~ 지점부터 입력창 시작
    ImGui::InputScalar("##particle_count", ImGuiDataType_U64, &cnt, NULL, NULL, "%zu");

    if (ImGui::IsItemDeactivatedAfterEdit()) // enter
    {
        if (auto* cmd = ctx.command_list->get<scene::Particle_change_command>()) {
            cmd->set_particle_count(cnt);
            cmd->execute();
            // FIXED : scene 멤버함수내에서 메세지를 발행해야하는데
            // scene 멤버함수가 ms_queue를 알아야함. 이게 싫다면 어케?
            ctx.msg_queue->Push_message<scene::Particle_change_message>(cnt,
                                                                        ctx.scene->data().entities.particles.data());
        }
    }
#endif
    // 1. 하단 배치를 위한 Y 커서 계산
    // 창 전체 높이 - (버튼 높이 + 여백)
    float button_height = ImGui::GetFrameHeightWithSpacing();
    float window_height = ImGui::GetWindowHeight();
    ImGui::SetCursorPosY(window_height - button_height - ImGui::GetStyle().WindowPadding.y);

    // 2. 우측 정렬을 위한 X 커서 계산
    float button_width = 120.0f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - button_width);

    // 3. 빨간색 스타일 적용
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));

    if (ImGui::Button("Reset particle", ImVec2(button_width, 0))) {
        ctx.msg_queue->Push_message<scene::Particle_change_message>(cnt, ctx.scene->data().entities.particles.data());
    }

    ImGui::PopStyleColor(3);
    ImGui::End();
}

void Imgui_renderer::show_camera_properties(Context& ctx)
{
    ImGui::SetNextWindowBgAlpha(0.35f);

    // 처음 실행될 때의 기본 크기 설정 (사용자가 조절하면 이 값은 무시됨)
    ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);

    // AlwaysAutoResize를 제거하여 사용자가 조절 가능하게 함
    if (!ImGui::Begin("Camera Control", nullptr)) {
        ImGui::End();
        return;
    }

    scene::Camera& cam = ctx.scene->camera();

    ImGui::TextDisabled("Left Drag rotate + WASD move");
    ImGui::SeparatorText("Transform");

    // Table을 사용하면 창 크기에 따라 X, Y, Z 폭이 자동으로 조절됨
    if (ImGui::BeginTable("##TransformTable", 3)) {
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        ImGui::DragFloat("X", &cam.data().transform.pos.x_, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopStyleColor();

        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
        ImGui::DragFloat("Y", &cam.data().transform.pos.y_, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopStyleColor();

        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 1.0f, 1.0f));
        ImGui::DragFloat("Z", &cam.data().transform.pos.z_, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopStyleColor();

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Movement");
    ImGui::Text("Speed");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN); // 남은 가로 공간을 모두 채움
    ImGui::SliderFloat("##CameraSpeed", &cam.data().transform.speed_scale, MIN_CAMERA_SPEED, MAX_CAMERA_SPEED,
                       "x %.2f");

    ImGui::Spacing();
    ImGui::SeparatorText("Frustum");

    ImGui::Text("Field of view");
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.4f); // 40% 지점부터 입력창 시작
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::DragFloat("##field_of_view", &cam.data().frustum.fov, 0.005f, MIN_CAMERA_FOV, MAX_CAMERA_FOV, "%.3f")) {
        cam.make_projection();
    }

    ImGui::Text("Near");
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.4f); // 40% 지점부터 입력창 시작
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::DragFloat("##near", &cam.data().frustum.near, 1.f, MIN_CAMERA_NEAR, MAX_CAMERA_NEAR, "%1.f")) {
        cam.make_projection();
    }
    ImGui::Text("Far");
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.4f); // 40% 지점부터 입력창 시작
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::DragFloat("##far", &cam.data().frustum.far, 10.f, MIN_CAMERA_FAR, MAX_CAMERA_FAR, "%1.f")) {
        cam.make_projection();
    }
    float fov{0.785f};       // 약 45도
    float aspect{1.777778f}; //
    float near{0.1f};
    float far{10000.0f};

    ImGui::Spacing();
    if (ImGui::Button("Reset View", ImVec2(-FLT_MIN, 0))) { // 가로로 꽉 찬 버튼
        cam.data().transform.pos = math::Vec3{0.0f, 0.0f, 500.0f};
        cam.data().transform.speed_scale = 1.0f;
    }

    ImGui::End();
}

void Imgui_renderer::show_device_data(Context& ctx)
{
    ImGui::SetNextWindowBgAlpha(0.25f);
    ImGui::Begin("Device setting");

    ImGui::SeparatorText("Compute Mode");
    ImGui::SetNextItemWidth(-FLT_MIN); // 남은 가로 공간을 모두 채움
    if (ImGui::Button("Gravity")) {
        ctx.device->set_compute_type(graphic::Compute_type::Gravity);
    }
    ImGui::SameLine();
    if (ImGui::Button("Electromagnetic")) {
        ctx.device->set_compute_type(graphic::Compute_type::Electromagnetic);
    }
    ImGui::SameLine();
    if (ImGui::Button("Time Varying EM Field")) {
        ctx.device->set_compute_type(graphic::Compute_type::Time_varying_EM_field);
    }
    ImGui::SeparatorText("Fade Effect");

    ImGui::Text("Fade Speed");
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.4f); // 40% 지점부터 입력창 시작
    ImGui::SetNextItemWidth(-FLT_MIN);

    if (ImGui::DragFloat("##fade_slider", &ctx.device->data().fade_scale, 0.005f, 0.001f, 1.0f, "%.3f",
                         ImGuiSliderFlags_Logarithmic)) {
    }

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
