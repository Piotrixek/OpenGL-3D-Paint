// Painter.cpp
#include "Painter.h"
#include "Shader.h" 
#include "Globals.h"  
#include "Camera.h"   
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glfw/glfw3.h>
#include <algorithm>
#include <vector>
#include <cmath> 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constructor
Painter::Painter() :
    drawing(false),
    brushAmbientColor(0.1f, 0.1f, 0.1f, 1.0f),
    brushDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f),
    brushSpecularColor(0.5f, 0.5f, 0.5f, 1.0f),
    brushShininess(32.0f),
    brushSize(2.0f),
    currentDrawStyle(FREEHAND),
    lightPos(1.0f, 5.0f, 3.0f),
    lightColor(1.0f, 1.0f, 1.0f),
    simpleVAO(0), simpleVBO(0),
    instancedVAO(0), instancedVBO(0), instanceDataVBO(0),
    cubeEBO(0), cubeIndexCount(0),
    sphereVAO(0), sphereVBO(0), sphereEBO(0), sphereIndexCount(0),
    tubeVAO(0), tubeVBO(0), tubeEBO(0),
    simpleShaderProgram(0), litShaderProgram(0)
{
    initShaders();
    initCube();       // For CUBE style (instanced)
    initSphere(16, 8); // For SPHERE style (instanced)
    initTubeResources(); // For TUBE style

    // VAO for simple line/point drawing
    glGenVertexArrays(1, &simpleVAO);
    glGenBuffers(1, &simpleVBO);
    glBindVertexArray(simpleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, simpleVBO);
    // Position attribute (simple)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0); // Unbind
}

// Destructor
Painter::~Painter() {
    glDeleteVertexArrays(1, &simpleVAO);
    glDeleteBuffers(1, &simpleVBO);
    glDeleteVertexArrays(1, &instancedVAO);
    glDeleteBuffers(1, &instancedVBO); // Base mesh VBO for instancing
    glDeleteBuffers(1, &instanceDataVBO); // Instance data VBO
    glDeleteBuffers(1, &cubeEBO);
    glDeleteVertexArrays(1, &sphereVAO); // Only if used for single sphere drawing
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteVertexArrays(1, &tubeVAO);
    glDeleteBuffers(1, &tubeVBO);
    glDeleteBuffers(1, &tubeEBO);

    glDeleteProgram(simpleShaderProgram);
    glDeleteProgram(litShaderProgram);
}

void Painter::initShaders() {
    // Keep the original simple shader if needed, or remove if all drawing uses lighting
    simpleShaderProgram = loadShader("shaders/paint.vert", "shaders/paint.frag");
    if (!simpleShaderProgram) {
        logger.addLog(" Failed to load simple paint shader!");
    }

    // Load the new shader with lighting
    litShaderProgram = loadShader("shaders/paint_lit.vert", "shaders/paint_lit.frag");
    if (!litShaderProgram) {
        logger.addLog(" Failed to load lit paint shader!");
    }
}

void Painter::initCube() {
    // Keep your existing cube vertices
    float vertices[] = {
        // positions          // normals (simple cube normals)
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    glGenVertexArrays(1, &instancedVAO); // Use a single VAO for all instanced meshes
    glGenBuffers(1, &instancedVBO);      // VBO for base mesh vertices (pos + normal)
    // No EBO needed for this cube definition (using triangles directly)
    cubeIndexCount = 36; // 6 faces * 2 triangles/face * 3 vertices/triangle

    glBindVertexArray(instancedVAO);

    glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Setup buffer and attributes for instance data (position, scale, color)
    setupInstancedVertexAttributes(instancedVAO, instanceDataVBO);

    glBindVertexArray(0); // Unbind VAO
}


void Painter::initSphere(int segments, int rings) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    vertices.clear();
    indices.clear();

    float radius = 0.5f; // Base radius for instancing

    for (int r = 0; r <= rings; ++r) {
        float phi = M_PI * (float)r / rings; // V texture coordinate equivalent
        float sinPhi = sin(phi);
        float cosPhi = cos(phi);

        for (int s = 0; s <= segments; ++s) {
            float theta = 2.0f * M_PI * (float)s / segments; // U texture coordinate equivalent
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            Vertex v;
            v.normal.x = cosTheta * sinPhi;
            v.normal.y = cosPhi;
            v.normal.z = sinTheta * sinPhi;
            v.position = v.normal * radius;
            // v.texCoords = glm::vec2((float)s / segments, (float)r / rings); // Add later

            vertices.push_back(v);
        }
    }

    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < segments; ++s) {
            int first = (r * (segments + 1)) + s;
            int second = first + segments + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
    sphereIndexCount = indices.size();

    // We already have the instancedVAO and instancedVBO. We just need to
    // potentially store sphere data if we want to draw it non-instanced,
    // or just remember the index count for instanced drawing.
    // For instanced drawing, we'll load the sphere data into the *same*
    // instancedVBO when the style switches to SPHERE.

    // Optional: If you need a separate VAO for drawing a single sphere
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    // TexCoords (Add later)
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    // glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}


void Painter::initTubeResources() {
    glGenVertexArrays(1, &tubeVAO);
    glGenBuffers(1, &tubeVBO);
    glGenBuffers(1, &tubeEBO);

    glBindVertexArray(tubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tubeVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tubeEBO); // Bind EBO here

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    // TexCoords attribute (Add later)
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    // glEnableVertexAttribArray(2);

    glBindVertexArray(0); // Unbind
}

// Helper to setup vertex attributes for instanced data (call *after* base mesh attributes)
void Painter::setupInstancedVertexAttributes(unsigned int VAO, unsigned int& instanceVBO) {
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    // Allocate buffer - size will be determined later when drawing
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW); // Start with zero size

    glBindVertexArray(VAO); // Bind the VAO we are adding attributes to

    // Instance Matrix (mat4) - spanning attribute locations 2, 3, 4, 5
    GLsizei vec4Size = sizeof(glm::vec4);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(1 * vec4Size));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * vec4Size));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * vec4Size));

    // Tell OpenGL this is per-instance data
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void Painter::addPoint(const glm::vec3& point) {
    if (!drawing) {
        drawing = true;
        currentStroke.points.clear();
        currentStroke.generatedVertices.clear(); // Clear generated geometry too
        currentStroke.generatedIndices.clear();

        // Store current brush settings with the stroke
        currentStroke.ambientColor = brushAmbientColor;
        currentStroke.diffuseColor = brushDiffuseColor;
        currentStroke.specularColor = brushSpecularColor;
        currentStroke.shininess = brushShininess;
        currentStroke.size = brushSize;
        currentStroke.style = currentDrawStyle; // Store the style used
    }
    currentStroke.points.push_back(point);

    // Optimization: For tubes, you could generate segments incrementally here
    // instead of all at once in endStroke, but it's more complex.
}

void Painter::endStroke() {
    if (drawing) {
        if (currentStroke.points.size() > 1) {
            // Smoothing happens on control points BEFORE geometry generation
            // smoothStroke(currentStroke); // Optional: Apply smoothing

            // Generate mesh if it's a style that needs it (like TUBE)
            if (currentStroke.style == TUBE) {
                generateTubeMesh(currentStroke);
            }
            // For instanced styles (CUBE, SPHERE), geometry is generated per-instance in draw call

            strokes.push_back(currentStroke);
            undoneStrokes.clear(); // Clear redo stack
        }
        drawing = false;
        // Clear current stroke temporary data
        currentStroke.points.clear();
        currentStroke.generatedVertices.clear();
        currentStroke.generatedIndices.clear();
    }
}


void Painter::generateTubeMesh(Stroke& stroke, int segments) {
    stroke.generatedVertices.clear();
    stroke.generatedIndices.clear();

    if (stroke.points.size() < 2) return;

    float radius = stroke.size * 0.05f; // Example: scale radius with brush size

    for (size_t i = 0; i < stroke.points.size(); ++i) {
        glm::vec3 p0 = stroke.points[i];
        glm::vec3 direction;

        // Calculate direction vector
        if (i < stroke.points.size() - 1) {
            direction = glm::normalize(stroke.points[i + 1] - p0);
        }
        else {
            // For the last point, use the direction from the previous segment
            direction = glm::normalize(p0 - stroke.points[i - 1]);
        }

        // Find a perpendicular vector (up vector)
        glm::vec3 arbitrary_up = (abs(direction.y) < 0.99f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(direction, arbitrary_up));
        glm::vec3 up = glm::normalize(glm::cross(right, direction)); // Re-orthogonalize

        // Generate vertices for the ring at this point
        for (int s = 0; s < segments; ++s) {
            float angle = 2.0f * M_PI * (float)s / segments;
            glm::vec3 normal = glm::normalize(cos(angle) * right + sin(angle) * up);
            glm::vec3 position = p0 + normal * radius;

            Vertex v;
            v.position = position;
            v.normal = normal;
            // v.texCoords = glm::vec2((float)s / segments, (float)i / (stroke.points.size() - 1)); // Add later
            stroke.generatedVertices.push_back(v);
        }
    }

    // Generate indices to connect the rings into a tube
    for (size_t i = 0; i < stroke.points.size() - 1; ++i) {
        for (int s = 0; s < segments; ++s) {
            int current_ring_start = i * segments;
            int next_ring_start = (i + 1) * segments;

            int p1 = current_ring_start + s;
            int p2 = current_ring_start + (s + 1) % segments; // Wrap around
            int p3 = next_ring_start + s;
            int p4 = next_ring_start + (s + 1) % segments; // Wrap around

            // Triangle 1
            stroke.generatedIndices.push_back(p1);
            stroke.generatedIndices.push_back(p3);
            stroke.generatedIndices.push_back(p2);


            // Triangle 2
            stroke.generatedIndices.push_back(p2);
            stroke.generatedIndices.push_back(p3);
            stroke.generatedIndices.push_back(p4);

        }
    }
}


void Painter::clear() {
    strokes.clear();
    currentStroke.points.clear();
    currentStroke.generatedVertices.clear();
    currentStroke.generatedIndices.clear();
    undoneStrokes.clear();
    drawing = false;
}

void Painter::undoStroke() {
    if (!strokes.empty()) {
        undoneStrokes.push_back(strokes.back());
        strokes.pop_back();
    }
}

void Painter::redoStroke() {
    if (!undoneStrokes.empty()) {
        strokes.push_back(undoneStrokes.back());
        undoneStrokes.pop_back();
    }
}

void Painter::smoothCurrentStroke() {
    // Smoothing only makes sense for the control points,
    // geometry needs regeneration afterwards if it's a mesh-based style.
    if (drawing && currentStroke.points.size() > 2) {
        smoothStroke(currentStroke); // Smooth control points
        // If it was a tube, regenerate mesh (or wait until endStroke)
        // if (currentStroke.style == TUBE) { generateTubeMesh(currentStroke); }
    }
}

void Painter::smoothStroke(Stroke& stroke) {
    if (stroke.points.size() < 3) return; // Need at least 3 points
    std::vector<glm::vec3> smoothed;
    smoothed.push_back(stroke.points.front()); // Keep first point
    for (size_t i = 1; i < stroke.points.size() - 1; ++i) {
        glm::vec3 prev = stroke.points[i - 1];
        glm::vec3 curr = stroke.points[i];
        glm::vec3 next = stroke.points[i + 1];
        glm::vec3 avg = (prev + curr * 2.0f + next) * 0.25f; // Weighted average
        smoothed.push_back(avg);
    }
    smoothed.push_back(stroke.points.back()); // Keep last point
    stroke.points = smoothed;
}


void Painter::updateSimpleBuffer(const std::vector<glm::vec3>& points) {
    if (points.empty()) return;
    glBindVertexArray(simpleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, simpleVBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_DYNAMIC_DRAW);
    // Vertex attrib pointer should already be set from init
    glBindVertexArray(0);
}

void Painter::updateTubeBuffers(const Stroke& stroke) {
    if (stroke.generatedVertices.empty() || stroke.generatedIndices.empty()) return;

    glBindVertexArray(tubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, tubeVBO);
    glBufferData(GL_ARRAY_BUFFER, stroke.generatedVertices.size() * sizeof(Vertex), stroke.generatedVertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, stroke.generatedIndices.size() * sizeof(unsigned int), stroke.generatedIndices.data(), GL_DYNAMIC_DRAW);

    // Vertex attrib pointers should already be set from init
    glBindVertexArray(0);
}


void Painter::draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos) {
    if (!litShaderProgram) return; // Don't draw if shader failed to load

    glUseProgram(litShaderProgram);

    // --- Set Uniforms ---
    // Matrices
    glUniformMatrix4fv(glGetUniformLocation(litShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(litShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Lighting
    glUniform3fv(glGetUniformLocation(litShaderProgram, "light.position"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(litShaderProgram, "light.color"), 1, glm::value_ptr(lightColor));
    // Example fixed light intensities - make these adjustable later
    glUniform3f(glGetUniformLocation(litShaderProgram, "light.ambient"), 0.2f, 0.2f, 0.2f);
    glUniform3f(glGetUniformLocation(litShaderProgram, "light.diffuse"), 0.8f, 0.8f, 0.8f); // Stronger diffuse
    glUniform3f(glGetUniformLocation(litShaderProgram, "light.specular"), 1.0f, 1.0f, 1.0f); // Full specular intensity

    // Camera Position (for specular)
    glUniform3fv(glGetUniformLocation(litShaderProgram, "viewPos"), 1, glm::value_ptr(viewPos));

    // --- Draw Completed Strokes ---
    for (const auto& stroke : strokes) {
        // Set Material properties for this stroke
        glUniform4fv(glGetUniformLocation(litShaderProgram, "material.ambient"), 1, glm::value_ptr(stroke.ambientColor));
        glUniform4fv(glGetUniformLocation(litShaderProgram, "material.diffuse"), 1, glm::value_ptr(stroke.diffuseColor));
        glUniform4fv(glGetUniformLocation(litShaderProgram, "material.specular"), 1, glm::value_ptr(stroke.specularColor));
        glUniform1f(glGetUniformLocation(litShaderProgram, "material.shininess"), stroke.shininess);

        // Determine how to draw based on the style stored *in the stroke*
        switch (stroke.style) {
        case FREEHAND:
            updateSimpleBuffer(stroke.points);
            glBindVertexArray(simpleVAO);
            glLineWidth(stroke.size); // Line width might not work well with lit shaders depending on GPU
            glUniformMatrix4fv(glGetUniformLocation(litShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f))); // Identity model
            glDrawArrays(GL_LINE_STRIP, 0, stroke.points.size());
            break;
        case POINTS:
            updateSimpleBuffer(stroke.points);
            glBindVertexArray(simpleVAO);
            glPointSize(stroke.size); // Point size might not work well with lit shaders
            glUniformMatrix4fv(glGetUniformLocation(litShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f))); // Identity model
            glDrawArrays(GL_POINTS, 0, stroke.points.size());
            break;
        case CUBE:
            drawStrokeInstanced(stroke, instancedVAO, cubeIndexCount, view, projection, viewPos);
            break;
        case SPHERE:
            // For sphere, we need to load sphere vertex data into the *instancedVBO*
            // This is slightly inefficient, ideally we'd have separate VBOs or use geometry shaders
            // Or better: have a dedicated sphere VAO/VBO and switch VAOs
            // Let's stick to reusing the instanced VBO for now.
            // If initSphere created sphereVBO/EBO:
            glBindBuffer(GL_ARRAY_BUFFER, instancedVBO); // Bind the base mesh VBO
            glBindVertexArray(sphereVAO); // Bind sphere VAO to get sphere vertex data
            glBindBuffer(GL_ARRAY_BUFFER, sphereVBO); // Get sphere data source
            GLint size; glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size); // Get sphere data size
            glBindBuffer(GL_ARRAY_BUFFER, instancedVBO); // Bind destination
            glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW); // Allocate space in instancedVBO
            glBindBuffer(GL_COPY_READ_BUFFER, sphereVBO);
            glBindBuffer(GL_COPY_WRITE_BUFFER, instancedVBO);
            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size); // Copy sphere data
            glBindBuffer(GL_COPY_READ_BUFFER, 0); glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

            // Now draw using the common instanced path
            drawStrokeInstanced(stroke, instancedVAO, sphereIndexCount, view, projection, viewPos);
            break;
        case TUBE:
            updateTubeBuffers(stroke);
            glBindVertexArray(tubeVAO);
            glUniformMatrix4fv(glGetUniformLocation(litShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f))); // Identity model
            glDrawElements(GL_TRIANGLES, stroke.generatedIndices.size(), GL_UNSIGNED_INT, 0);
            break;
        }
    }
    glBindVertexArray(0); // Unbind VAO after drawing all strokes


    // --- Draw Current Stroke (Preview) ---
    if (drawing && currentStroke.points.size() > 0) {
        // Set Material properties for the current brush
        glUniform4fv(glGetUniformLocation(litShaderProgram, "material.ambient"), 1, glm::value_ptr(brushAmbientColor));
        glUniform4fv(glGetUniformLocation(litShaderProgram, "material.diffuse"), 1, glm::value_ptr(brushDiffuseColor));
        glUniform4fv(glGetUniformLocation(litShaderProgram, "material.specular"), 1, glm::value_ptr(brushSpecularColor));
        glUniform1f(glGetUniformLocation(litShaderProgram, "material.shininess"), brushShininess);

        glm::mat4 identity = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(litShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(identity)); // Reset model matrix for non-instanced

        switch (currentDrawStyle) {
        case FREEHAND:
            if (currentStroke.points.size() > 1) {
                updateSimpleBuffer(currentStroke.points);
                glBindVertexArray(simpleVAO);
                glLineWidth(brushSize);
                glDrawArrays(GL_LINE_STRIP, 0, currentStroke.points.size());
            }
            break;
        case POINTS:
            updateSimpleBuffer(currentStroke.points);
            glBindVertexArray(simpleVAO);
            glPointSize(brushSize);
            glDrawArrays(GL_POINTS, 0, currentStroke.points.size());
            break;
        case CUBE:
            // Previewing instanced strokes requires drawing one instance at the last point
            if (!currentStroke.points.empty()) {
                glm::mat4 model = glm::translate(identity, currentStroke.points.back());
                model = glm::scale(model, glm::vec3(brushSize * 0.1f)); // Apply scaling
                glUniformMatrix4fv(glGetUniformLocation(litShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glBindVertexArray(instancedVAO); // Use the base cube VAO
                glDrawArrays(GL_TRIANGLES, 0, cubeIndexCount); // Draw one cube
            }
            break;
        case SPHERE:
            // Previewing instanced strokes requires drawing one instance at the last point
            if (!currentStroke.points.empty()) {
                glm::mat4 model = glm::translate(identity, currentStroke.points.back());
                model = glm::scale(model, glm::vec3(brushSize * 0.1f)); // Apply scaling
                glUniformMatrix4fv(glGetUniformLocation(litShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glBindVertexArray(sphereVAO); // Use the base sphere VAO
                glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0); // Draw one sphere
            }
            break;
        case TUBE:
            // Previewing tubes is tricky - generate mesh for current stroke?
            // For simplicity, maybe just draw lines for preview
            if (currentStroke.points.size() > 1) {
                // Option 1: Draw lines as preview
                updateSimpleBuffer(currentStroke.points);
                glBindVertexArray(simpleVAO);
                glLineWidth(brushSize);
                glDrawArrays(GL_LINE_STRIP, 0, currentStroke.points.size());

                // Option 2: Generate and draw tube preview (can be slow)
                // Stroke previewTube = currentStroke; // Copy
                // generateTubeMesh(previewTube);
                // updateTubeBuffers(previewTube);
                // glBindVertexArray(tubeVAO);
                // glDrawElements(GL_TRIANGLES, previewTube.generatedIndices.size(), GL_UNSIGNED_INT, 0);
            }
            break;
        }
        glBindVertexArray(0); // Unbind after preview
    }
}

// Helper function for drawing instanced strokes (CUBE, SPHERE)
void Painter::drawStrokeInstanced(const Stroke& stroke, unsigned int baseVAO, int indexCount, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos)
{
    if (stroke.points.empty()) return;

    // 1. Prepare instance data (Model matrices)
    std::vector<glm::mat4> instanceModels;
    instanceModels.reserve(stroke.points.size());
    for (const auto& point : stroke.points) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, point);
        // Adjust scale based on stroke size and maybe style
        float scaleFactor = (stroke.style == CUBE || stroke.style == SPHERE) ? stroke.size * 0.1f : stroke.size;
        model = glm::scale(model, glm::vec3(scaleFactor));
        instanceModels.push_back(model);
    }

    // 2. Upload instance data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, instanceDataVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceModels.size() * sizeof(glm::mat4), instanceModels.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // 3. Bind base mesh VAO (already contains vertex attributes for base mesh AND instance data)
    glBindVertexArray(baseVAO);

    // 4. Set uniforms (view, projection, light, material - already set in draw())
    // No need to set model uniform here, it's handled by instance attributes

    // 5. Draw instanced
    if (stroke.style == CUBE) { // Cube uses glDrawArrays
        glDrawArraysInstanced(GL_TRIANGLES, 0, indexCount, stroke.points.size());
    }
    else if (stroke.style == SPHERE) { // Sphere uses glDrawElements
        // Ensure sphere EBO is bound within its VAO setup
        glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, stroke.points.size());
    }


    glBindVertexArray(0);
}


// --- Other Painter methods ---

void Painter::setBrushDiffuseColor(const glm::vec4& color) {
    brushDiffuseColor = color;
    // Update current stroke preview if drawing
    if (drawing) currentStroke.diffuseColor = brushDiffuseColor;
}

void Painter::setBrushSize(float size) {
    brushSize = size;
    if (drawing) currentStroke.size = brushSize;
}

int Painter::getStrokeCount() const {
    return strokes.size(); // Don't count the current stroke being drawn
}

void Painter::removeLastPoint() {
    if (drawing && !currentStroke.points.empty()) {
        currentStroke.points.pop_back();
        // Need to regenerate mesh if it's TUBE style and we want accurate preview
    }
}

// Scale/Translate affect control points. Mesh needs regenerating for tubes.
void Painter::scaleCurrentStroke(float scaleFactor) {
    if (drawing && !currentStroke.points.empty()) {
        glm::vec3 center(0.0f);
        for (const auto& p : currentStroke.points)
            center += p;
        center /= currentStroke.points.size();
        for (auto& p : currentStroke.points)
            p = center + (p - center) * scaleFactor;
        // Need to regenerate mesh if it's TUBE style and we want accurate preview
    }
}

void Painter::translateCurrentStroke(const glm::vec3& translation) {
    if (drawing && !currentStroke.points.empty()) {
        for (auto& p : currentStroke.points)
            p += translation;
        // Need to regenerate mesh if it's TUBE style and we want accurate preview
    }
}

void Painter::duplicateLastStroke() {
    if (!strokes.empty()) {
        strokes.push_back(strokes.back());
        // Ensure deep copy if necessary (vectors are copied by value here)
    }
}

void Painter::mergeAllStrokes() {
    if (strokes.size() < 2) return; // Need at least two strokes to merge

    Stroke merged;
    // Use properties of the first stroke as base? Or current brush? Let's use current brush.
    merged.ambientColor = brushAmbientColor;
    merged.diffuseColor = brushDiffuseColor;
    merged.specularColor = brushSpecularColor;
    merged.shininess = brushShininess;
    merged.size = brushSize;
    merged.style = currentDrawStyle; // Merged stroke gets the current style? Or maybe FREEHAND?

    // Collect all control points
    for (const auto& stroke : strokes) {
        merged.points.insert(merged.points.end(), stroke.points.begin(), stroke.points.end());
    }

    // Regenerate geometry if needed for the merged stroke's style
    if (merged.style == TUBE) {
        generateTubeMesh(merged);
    }

    strokes.clear();
    strokes.push_back(merged);
    undoneStrokes.clear(); // Cannot undo a merge easily
}


void Painter::clearUndoneStrokes() {
    undoneStrokes.clear();
}

void Painter::reverseCurrentStroke() {
    if (drawing && currentStroke.points.size() > 1) {
        std::reverse(currentStroke.points.begin(), currentStroke.points.end());
        // Need to regenerate mesh if it's TUBE style and we want accurate preview
    }
}

void Painter::setDrawStyle(DrawStyle style) {
    currentDrawStyle = style;
    // If switching to an instanced style, potentially reload the base mesh into instancedVBO
    if (style == SPHERE) {
        // Copy sphere data to the instancedVBO (as done in draw() currently)
        // This logic might be better placed here or in initSphere/initCube
    }
    else if (style == CUBE) {
        // Copy cube data to the instancedVBO
    }
}

Painter::DrawStyle Painter::getDrawStyle() const {
    return currentDrawStyle;
}