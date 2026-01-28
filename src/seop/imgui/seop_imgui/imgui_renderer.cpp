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
        ctx.device->set_frame_rate(data.frame_rate);
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

        make_sub_title("Amplitude");
        ImGui::SliderFloat("##max_current", &data.wire_properites.max_i, 0.0f, 20.0f, "%.1f");

        make_sub_title("Angle Frequency");
        ImGui::DragFloat("##angle_frequency", &data.wire_properites.w, 1.0f, 1.0f, 200.0f, "%1.f",
                         ImGuiSliderFlags_Logarithmic);

        make_sub_title("Light Velocity");
        ImGui::DragFloat("##light_velociy", &data.wire_properites.c, 1.0f, 1.0f, 2000.0f, "%1.f",
                         ImGuiSliderFlags_Logarithmic);

#if 1
        // 1. 데이터를 담을 배열 준비 (예: 100개 포인트)
        float values[100];
        for (int n = 0; n < 100; n++) {
            values[n] = data.wire_properites.max_i *
                        sinf(data.wire_properites.w * (n * ctx.f_dt + static_cast<float>(ctx.d_time)));
        }

        // 2. ImGui 창 안에서 그리기
        ImGui::PlotLines("Sine Wave", values, IM_ARRAYSIZE(values), 0, "Overlay Text", -20.0f, 20.0f, ImVec2(0, 80));
#endif
    }
    if (ImGui::Button("Add wire")) {
        ctx.scene->add_wire();
        ctx.msg_queue->Push_message<scene::Wire_buffer_update_message>(
            ctx.scene->data().wire_properites.wire_nodes.vb.buf_.id_,
            sizeof(primitive::Vertex_pcs) * ctx.scene->data().wire_properites.wire_nodes.vb.vertices.size(),
            ctx.scene->data().wire_properites.wire_nodes.vb.vertices.data());
    }

    ImGui::End();
}

auto Imgui_renderer::hovering_ui() -> bool
{
    return ImGui::GetIO().WantCaptureMouse;
}

void Imgui_renderer::make_title(const std::string& title)
{
    ImGui::Spacing();
    ImGui::SeparatorText(title.c_str());
}

void Imgui_renderer::make_sub_title(const std::string& title)
{
    ImGui::Text(title.c_str());
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN); // 남은 가로 공간을 모두 채움
}

void Imgui_renderer::show_particle_properties(Context& ctx)
{
    ImGui::SetNextWindowBgAlpha(0.25f);
    if (!ImGui::Begin("Particle Option", nullptr)) {
        ImGui::End();
        return;
    }

    scene::Particle_propetires& properties = ctx.scene->data().particle_properties;

    make_title("Particle Speed");
    make_sub_title("Speed");
    ImGui::DragFloat("##particle_speed", &properties.time_scale, 0.005f, -5.0f, 10.0f, "%.1f");

    make_title("Particle size");
    make_sub_title("size");
    ImGui::SliderFloat("##particle_size", &properties.size, 1.0f, 10.0f, "%1.0f");

    make_title("Particle Color");
    make_sub_title("Color");
    ImGui::SliderFloat("##particle_color", &properties.col, 0.0f, 1.0f, "%.3f");

#if 1
    size_t cnt = properties.count;
    make_title("Particle Count");
    make_sub_title("Count");
    ImGui::InputScalar("##particle_count", ImGuiDataType_U64, &cnt, NULL, NULL, "%zu");

    if (ImGui::IsItemDeactivatedAfterEdit()) // enter
    {
        if (auto* cmd = ctx.command_list->get<scene::Particle_change_command>()) {
            cmd->set_particle_count(cnt);
            cmd->execute();
            // FIXED : scene 멤버함수내에서 메세지를 발행해야하는데
            // scene 멤버함수가 ms_queue를 알아야함. 이게 싫다면 어케?
            ctx.msg_queue->Push_message<scene::Particle_buffer_update_message>(
                ctx.scene->data().entities.particles.vb.buf_.id_, sizeof(primitive::Vertex_pcv) * cnt,
                ctx.scene->data().entities.particles.vb.vertices.data());
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
        ctx.msg_queue->Push_message<scene::Particle_buffer_update_message>(
            ctx.scene->data().entities.particles.vb.buf_.id_, sizeof(primitive::Vertex_pcv) * cnt,
            ctx.scene->data().entities.particles.vb.vertices.data());
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

    make_title("Movement");
    make_sub_title("Speed");
    ImGui::SliderFloat("##CameraSpeed", &cam.data().transform.speed_scale, MIN_CAMERA_SPEED, MAX_CAMERA_SPEED,
                       "x %1.f");

    make_title("Frustum");
    make_sub_title("FOV");
    if (ImGui::DragFloat("##field_of_view", &cam.data().frustum.fov, 0.005f, MIN_CAMERA_FOV, MAX_CAMERA_FOV, "%.3f")) {
        cam.make_projection();
    }

    make_sub_title("Near");
    if (ImGui::DragFloat("##near", &cam.data().frustum.near, 1.f, MIN_CAMERA_NEAR, MAX_CAMERA_NEAR, "%1.f")) {
        cam.make_projection();
    }

    make_sub_title("Far");
    if (ImGui::DragFloat("##far", &cam.data().frustum.far, 10.f, MIN_CAMERA_FAR, MAX_CAMERA_FAR, "%1.f")) {
        cam.make_projection();
    }

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

    // -------------------------------------------------------------------------------
    make_title("Compute Type");
    if (ImGui::Button("Lolernz Equation")) {
        ctx.device->set_compute_type(graphic::Compute_type::Lorenz_equation);
    }
    // -------------------------------------------------------------------------------
    ImGui::SameLine();
    if (ImGui::Button("Time Varying EM Field")) {
        ctx.device->set_compute_type(graphic::Compute_type::Time_varying_EM_field);
    }
    // -------------------------------------------------------------------------------

    make_title("Fade Setting");
    make_sub_title("speed");

    if (ImGui::DragFloat("##fade_slider", &ctx.device->data().fade_scale, 0.005f, 0.001f, 1.0f, "%.3f",
                         ImGuiSliderFlags_Logarithmic)) {
    }
    // -------------------------------------------------------------------------------

    make_title("Mode");
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    if (ImGui::Button("Dark Mode")) {
        ctx.device->set_back_col(math::Vec4{0.0f, 0.0f, 0.0f, 1.0f});
    }
    ImGui::PopStyleColor(3);
    // -------------------------------------------------------------------------------

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.9f, 0.9f, 1.0f)); // button
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // text
    if (ImGui::Button("White Mode")) {
        ctx.device->set_back_col(math::Vec4{1.0f, 1.0f, 1.0f, 1.0f});
    }
    ImGui::PopStyleColor(4);
    // -------------------------------------------------------------------------------

    make_title("View Field Setting");
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.5f, 0.5f, 1.0f)); // button
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // text
    if (ImGui::Button("Electric Field Arrow")) {
        if (ctx.device->view_electric_field_arrow) {
            ctx.device->view_electric_field_arrow = (false);
        } else {
            ctx.device->view_electric_field_arrow = (true);
        }
    }
    ImGui::PopStyleColor(3);
    // -------------------------------------------------------------------------------
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 1.0f, 1.0f)); // button
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // text
    if (ImGui::Button("Magnetic Field Arrow")) {
        if (ctx.device->view_magnetic_field_arrow) {
            ctx.device->view_magnetic_field_arrow = (false);
        } else {
            ctx.device->view_magnetic_field_arrow = (true);
        }
    }
    ImGui::PopStyleColor(3);

    // -------------------------------------------------------------------------------

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.9f, 0.4f, .9f, 1.0f)); // button
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.8f, 0.3f, 1.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // text
    if (ImGui::Button("Poynting Field Arrow")) {
        if (ctx.device->view_poynting_field_arrow) {
            ctx.device->view_poynting_field_arrow = (false);
        } else {
            ctx.device->view_poynting_field_arrow = (true);
        }
    }
    ImGui::PopStyleColor(3);

    // -------------------------------------------------------------------------------
    make_sub_title("Arrow Scale");
    ImGui::SliderFloat("##arrow_scale", &ctx.device->arrow_scale, 1.0f, 2000.0f, "%1.f");
    // -------------------------------------------------------------------------------
    make_sub_title("Arrow Tickness");
    ImGui::SliderFloat("##arrow_tickness", &ctx.device->arrow_thickness, 1.0f, 10.0f, "%1.f");

    // -------------------------------------------------------------------------------
    auto update_arrows = [&]() {
        ctx.device->create_arrow(ctx.device->arrow_interval, ctx.device->arrow_range);
        ctx.device->update_shader_buffer(ctx.device->arrow_nodes_.vb.buf_.id_, 2,
                                         sizeof(primitive::Vertex_pf) * ctx.device->arrow_nodes_.vb.vertices.size(),
                                         ctx.device->arrow_nodes_.vb.vertices.data());
    };

    make_sub_title("Arrow Interval");
    if (ImGui::SliderInt("##arrow_interval", &ctx.device->arrow_interval, 40, 1000)) {
        update_arrows();
    }
    // -------------------------------------------------------------------------------
    bool changed = false;
    make_sub_title("Min X Range");
    if (ImGui::InputInt("##arrow_min_range_x", &ctx.device->arrow_range[0])) {
        update_arrows();
    }
    make_sub_title("Max X Range");
    if (ImGui::InputInt("##arrow_max_range_x", &ctx.device->arrow_range[1])) {
        update_arrows();
    }
    make_sub_title("Min Y Range");
    if (ImGui::InputInt("##arrow_min_range_y", &ctx.device->arrow_range[2])) {
        update_arrows();
    }
    make_sub_title("Max Y Range");
    if (ImGui::InputInt("##arrow_max_range_y", &ctx.device->arrow_range[3])) {
        update_arrows();
    }
    make_sub_title("Min Z Range");
    if (ImGui::InputInt("##arrow_min_range_z", &ctx.device->arrow_range[4])) {
        update_arrows();
    }
    make_sub_title("Max Z Range");
    if (ImGui::InputInt("##arrow_max_range_z", &ctx.device->arrow_range[5])) {
        update_arrows();
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
