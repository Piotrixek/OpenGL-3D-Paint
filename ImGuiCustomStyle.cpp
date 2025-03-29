#include "ImGuiCustomStyle.h"
#include "imgui.h"

void SetupCustomImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 8.0f;
    style.FrameRounding = 12.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 12.0f;

    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(12, 8);
    style.ItemSpacing = ImVec2(10, 10);
    style.ItemInnerSpacing = ImVec2(8, 8);

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_PopupBg] = ImVec4(0, 0, 0, 0.9f); 

    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.7f, 1.0f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.1f, 0.5f, 0.9f, 1.0f);

    colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);

    colors[ImGuiCol_FrameBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.5f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.2f, 0.2f, 0.7f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);

}
