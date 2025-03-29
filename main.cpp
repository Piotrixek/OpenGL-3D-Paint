#define GLFW_EXPOSE_NATIVE_WIN32
#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Camera.h"
#include "Logger.h"
#include "Shader.h"
#include "Callbacks.h"
#include "Globals.h"
#include "Painter.h"
#include "Util.h"
#include "ImGuiCustomStyle.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Simulation", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    unsigned int skyVAO, skyVBO;
    float skyVertices[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
    glGenVertexArrays(1, &skyVAO);
    glGenBuffers(1, &skyVBO);
    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyVertices), skyVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    unsigned int skyShader = loadShader("shaders/sky.vert", "shaders/sky.frag");
    if (!skyShader) {
        std::cerr << "Critical shader load error!\n";
        glfwTerminate();
        return -1;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    SetupCustomImGuiStyle();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    Painter painter;
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 projection = camera.getProjectionMatrix(1280.0f / 720.0f);
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(skyShader);
        glBindVertexArray(skyVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glEnable(GL_DEPTH_TEST);
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                glm::vec3 worldPos = screenToWorld(window, (float)mouseX, (float)mouseY, camera, 5.0f);
                painter.addPoint(worldPos);
            }
            else {
                painter.endStroke();
            }
        }
        else {
            painter.endStroke();
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Controls");
        ImGui::ColorEdit4("Brush Color", &painter.brushColor[0]);
        ImGui::SliderFloat("Brush Size", &painter.brushSize, 1.0f, 10.0f);
        const char* styles[] = { "Freehand", "Cube", "Points" };
        int currentStyle = static_cast<int>(painter.getDrawStyle());
        if (ImGui::Combo("Draw Style", &currentStyle, styles, IM_ARRAYSIZE(styles)))
            painter.setDrawStyle(static_cast<Painter::DrawStyle>(currentStyle));
        if (ImGui::Button("Clear")) painter.clear();
        if (ImGui::Button("Undo")) painter.undoStroke();
        if (ImGui::Button("Redo")) painter.redoStroke();
        if (ImGui::Button("Smooth Current")) painter.smoothCurrentStroke();
        if (ImGui::Button("Remove Last Point")) painter.removeLastPoint();
        if (ImGui::Button("Scale Current (x1.1)")) painter.scaleCurrentStroke(1.1f);
        if (ImGui::Button("Translate Current (+0.1,0,0)")) painter.translateCurrentStroke(glm::vec3(0.1f, 0.0f, 0.0f));
        if (ImGui::Button("Duplicate Last Stroke")) painter.duplicateLastStroke();
        if (ImGui::Button("Merge All Strokes")) painter.mergeAllStrokes();
        if (ImGui::Button("Clear Undone")) painter.clearUndoneStrokes();
        if (ImGui::Button("Reverse Current")) painter.reverseCurrentStroke();
        ImGui::Text("Total Strokes: %d", painter.getStrokeCount());
        ImGui::End();
        painter.draw(view, projection);
        logger.draw("Application Log");
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteVertexArrays(1, &skyVAO);
    glDeleteBuffers(1, &skyVBO);
    glfwTerminate();
    return 0;
}
