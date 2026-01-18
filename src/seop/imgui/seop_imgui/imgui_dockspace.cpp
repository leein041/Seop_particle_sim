#include "imgui_dockspace.hpp"

#include "imgui/imgui.h"

namespace seop::imgui
{
void Dockspace::init()
{
    ImGuiIO& io = ImGui::GetIO();

    // 도킹 기능 활성화
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // 창을 메인 윈도우 밖으로 빼고 싶다면 이것도 활성화 (에러 해결했으니 안심하고 쓰셔도 됩니다)
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
}

void Dockspace::begin()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // 추가: 윈도우 스타일에서 Rounding과 Border를 0으로 밀어버림 (전체화면용)
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); // 여백 제거

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |=
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    // 배경 투명색 설정
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    bool p_open = true;
    ImGui::Begin("MyMainDockSpace", &p_open, window_flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3); // PushStyleVar 3번 한 거 해제

    // 핵심: PassthruCentralNode 플래그 추가
    ImGuiID            dockspace_id = ImGui::GetID("MainDockSpace");
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
}

void Dockspace::end()
{
    ImGui::End();
}
} // namespace seop::imgui
