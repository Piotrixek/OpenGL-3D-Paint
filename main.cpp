#define GLFW_EXPOSE_NATIVE_WIN32
#include <windows.h>             // Keep for WinMain
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// headers
#include "Camera.h"           
#include "Logger.h"           
#include "Shader.h"         
#include "Callbacks.h"        
#include "Globals.h"        
#include "Painter.h"          
#include "Util.h"            
#include "ImGuiCustomStyle.h"

/*
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
Logger logger;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool mouseCaptured = true;
*/

// Screen dimensions
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // --- GLFW Initialization ---
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
#endif

    // --- GLFW Window Creation ---
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Painter", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // --- GLAD Initialization ---
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // --- OpenGL Global State ---
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND); // Enable blending for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE); // Enable if using gl_PointSize in shaders for POINTS style
    // think of um enabling face culling for performance with solid objects
    // glEnable(GL_CULL_FACE);

    // --- Setup Callbacks ---
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // Initial mouse mode setup (from Callbacks.cpp logic)
    glfwSetInputMode(window, GLFW_CURSOR, mouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    if (mouseCaptured) camera.firstMouse = true; // Reset firstMouse flag if starting captured


    // --- Skybox Setup ---
    unsigned int skyVAO, skyVBO;
    // sky vertices (quad covering screen)
    float skyVertices[] = {
        // positions
        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f,  1.0f,
         1.0f, -1.0f,
    };
    glGenVertexArrays(1, &skyVAO);
    glGenBuffers(1, &skyVBO);
    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyVertices), skyVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0); // Unbind skyVAO

  
    unsigned int skyShader = loadShader("shaders/sky.vert", "shaders/sky.frag");
    if (!skyShader) {
        logger.addLog("[CRITICAL] Failed to load skybox shader!");
        // ee could terminate if skybox is critical
    }


    // --- ImGui Initialization ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io; // Basic IO setup
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Optional: Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Optional: Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Optional: Enable Multi-Viewport

    ImGui::StyleColorsDark();
    SetupCustomImGuiStyle(); // custom style function

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param installs callbacks
    ImGui_ImplOpenGL3_Init("#version 330");


    // --- Create Painter ---
    Painter painter;


    // --- Main Render Loop ---
    while (!glfwWindowShouldClose(window)) {
        // --- Per-frame Time Logic ---
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // --- Input Processing ---
        processInput(window); // Handle keyboard input (camera movement, exit, mouse capture toggle)

        // --- Rendering ---
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Set background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Camera Matrices ---
        // Get window size dynamically for aspect ratio (better than fixed values)
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        float aspectRatio = (display_h > 0) ? (float)display_w / (float)display_h : 1.0f;

        glm::mat4 projection = camera.getProjectionMatrix(aspectRatio);
        glm::mat4 view = camera.getViewMatrix();
        glm::vec3 viewPos = camera.position; // Get camera position for lighting

        // --- Draw Skybox (First, behind everything else) ---
        if (skyShader) { // Check if shader loaded successfully
            glDepthMask(GL_FALSE); // Disable depth writing for skybox
            glUseProgram(skyShader);
            // Pass uniforms if your sky shader needs them (e.g., view/projection without translation)
            // glm::mat4 skyView = glm::mat4(glm::mat3(view)); // Remove translation
            // glUniformMatrix4fv(glGetUniformLocation(skyShader, "view"), 1, GL_FALSE, &skyView[0][0]);
            // glUniformMatrix4fv(glGetUniformLocation(skyShader, "projection"), 1, GL_FALSE, &projection[0][0]);
            glBindVertexArray(skyVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glDepthMask(GL_TRUE); // Re-enable depth writing
            glBindVertexArray(0);
        }


        // --- Handle Painting Input (Screen to World) ---
        // Only process painting if ImGui doesn't want the mouse AND mouse is captured for camera control
        if (!io.WantCaptureMouse && mouseCaptured) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY); // Get current mouse position

            // Check if the left mouse button is pressed
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                // Determine the distance from the camera to place the point
                // This could be fixed, based on camera target, or raycast result later
                float paintDistance = 5.0f; // Example fixed distance
                glm::vec3 worldPos = screenToWorld(window, (float)mouseX, (float)mouseY, camera, paintDistance);
                painter.addPoint(worldPos); // Add the point to the current stroke
            }
            else {
                painter.endStroke(); // Finish the current stroke if mouse button is released
            }
        }
        else {
            // If ImGui wants the mouse OR the mouse isn't captured for camera control, ensure stroke ends
            painter.endStroke();
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        // --- ImGui Controls Window ---
        ImGui::Begin("Controls"); 

        // --- Draw Style Selection ---
        ImGui::Text("Drawing Style");
        const char* styles[] = { "Freehand", "Cube", "Points", "Sphere", "Tube" };
        int currentStyle = static_cast<int>(painter.getDrawStyle());
        if (ImGui::Combo("##DrawStyle", &currentStyle, styles, IM_ARRAYSIZE(styles))) { // Use ## for hidden label
            painter.setDrawStyle(static_cast<Painter::DrawStyle>(currentStyle));
        }
        ImGui::Separator();

        // --- Material Properties ---
        ImGui::Text("Brush Material");
        ImGui::ColorEdit4("Ambient##Brush", glm::value_ptr(painter.brushAmbientColor));
        ImGui::ColorEdit4("Diffuse##Brush", glm::value_ptr(painter.brushDiffuseColor)); 
        ImGui::ColorEdit4("Specular##Brush", glm::value_ptr(painter.brushSpecularColor));
        ImGui::SliderFloat("Shininess##Brush", &painter.brushShininess, 2.0f, 256.0f);
        ImGui::Separator();

        // --- Brush Size ---
        ImGui::Text("Brush Settings");
        ImGui::SliderFloat("Size##Brush", &painter.brushSize, 0.1f, 10.0f);
        ImGui::Separator();

        // --- Stroke Operations ---
        ImGui::Text("Stroke Operations");
        if (ImGui::Button("Clear")) painter.clear();
        ImGui::SameLine();
        if (ImGui::Button("Undo")) painter.undoStroke();
        ImGui::SameLine();
        if (ImGui::Button("Redo")) painter.redoStroke();

        if (ImGui::Button("Smooth Current")) painter.smoothCurrentStroke();
        ImGui::SameLine();
        if (ImGui::Button("Remove Last Pt")) painter.removeLastPoint();

        if (ImGui::Button("Scale Curr (x1.1)")) painter.scaleCurrentStroke(1.1f);
        ImGui::SameLine();
        if (ImGui::Button("Trans Curr (+X)")) painter.translateCurrentStroke(glm::vec3(0.1f, 0.0f, 0.0f));

        if (ImGui::Button("Duplicate Last")) painter.duplicateLastStroke();
        ImGui::SameLine();
        if (ImGui::Button("Merge All")) painter.mergeAllStrokes();

        if (ImGui::Button("Clear Undone")) painter.clearUndoneStrokes();
        ImGui::SameLine();
        if (ImGui::Button("Reverse Current")) painter.reverseCurrentStroke();
        ImGui::Separator();


        // --- Light Controls ---
        ImGui::Text("Scene Light");
        ImGui::DragFloat3("Position##Light", glm::value_ptr(painter.lightPos), 0.1f);
        ImGui::ColorEdit3("Color##Light", glm::value_ptr(painter.lightColor));
        ImGui::Separator();


        // --- Info ---
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Total Strokes: %d", painter.getStrokeCount());

        ImGui::End(); // End Controls Window


        // --- Draw Painter Content ---
        // Pass camera position needed for lighting calculations in the shader
        painter.draw(view, projection, viewPos);


        // --- Draw Logger Window ---
        logger.draw("Application Log"); // Draw your log window


        // --- Render ImGui ---
        ImGui::Render(); // Assemble draw data
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Render draw data

        // --- Handle ImGui Viewports (Optional) ---
        // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        //     GLFWwindow* backup_current_context = glfwGetCurrentContext();
        //     ImGui::UpdatePlatformWindows();
        //     ImGui::RenderPlatformWindowsDefault();
        //     glfwMakeContextCurrent(backup_current_context);
        // }


        // --- GLFW Swap Buffers and Poll Events ---
        glfwSwapBuffers(window); // Swap the front and back buffers
        glfwPollEvents();      // Check for and process events
    }

    // --- Cleanup ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Painter destructor cleans up its own OpenGL objects when 'painter' goes out of scope
    // Cleanup skybox resources
    glDeleteVertexArrays(1, &skyVAO);
    glDeleteBuffers(1, &skyVBO);
    if (skyShader) glDeleteProgram(skyShader); // Delete shader if loaded

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0; // Successful exit
}

