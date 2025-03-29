#include "ImGuiCustomStyle.h"
#include "imgui.h"

void SetupCustomImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Set rounded corners for windows, frames, popups, etc.
    style.WindowRounding = 8.0f;
    style.FrameRounding = 12.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 12.0f;

    // Set padding and spacing for a more spacious, modern look.
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(12, 8);
    style.ItemSpacing = ImVec2(10, 10);
    style.ItemInnerSpacing = ImVec2(8, 8);

    // Hide window background (transparent) so only controls are visible.
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_PopupBg] = ImVec4(0, 0, 0, 0.9f); // Keep popups slightly opaque

    // Customize button colors with a modern blue palette.
    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.7f, 1.0f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.1f, 0.5f, 0.9f, 1.0f);

    // Customize text color and add more contrast.
    colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);

    // Adjust frame background to be subtle.
    colors[ImGuiCol_FrameBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.5f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.2f, 0.2f, 0.7f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);

    // Increase font size for better readability if desired.
    // (You can also load a custom font using ImGuiIO::Fonts if you want to go further.)
}
