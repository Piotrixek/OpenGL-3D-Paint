// Painter.h
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "glad/glad.h"
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector> 

// Forward declaration
class Camera;

class Painter {
public:
    // Added SPHERE and TUBE styles
    enum DrawStyle { FREEHAND, CUBE, POINTS, SPHERE, TUBE };

    Painter();
    ~Painter(); // Add destructor to clean up resources

    void addPoint(const glm::vec3& point);
    void endStroke();
    void clear();
    // Pass camera position for specular lighting
    void draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos);
    void undoStroke();
    void redoStroke();
    void smoothCurrentStroke();

    // Renamed to clarify it sets the diffuse color
    void setBrushDiffuseColor(const glm::vec4& color);
    void setBrushSize(float size);
    int getStrokeCount() const;
    void removeLastPoint();
    void scaleCurrentStroke(float scaleFactor);
    void translateCurrentStroke(const glm::vec3& translation);
    void duplicateLastStroke();
    void mergeAllStrokes();
    void clearUndoneStrokes();
    void reverseCurrentStroke();
    void setDrawStyle(DrawStyle style);
    DrawStyle getDrawStyle() const;

    // --- Material Properties ---
    glm::vec4 brushAmbientColor;
    glm::vec4 brushDiffuseColor; // Renamed from brushColor
    glm::vec4 brushSpecularColor;
    float brushShininess;
    // --- Brush Size ---
    float brushSize;
    // --- Light Properties ---
    glm::vec3 lightPos;
    glm::vec3 lightColor;


private:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        // Add texCoords here later for texturing
        // glm::vec2 texCoords;
    };

    struct Stroke {
        std::vector<glm::vec3> points; // Original control points
        std::vector<Vertex> generatedVertices; // Vertices for rendering (e.g., tube mesh)
        std::vector<unsigned int> generatedIndices; // Indices for rendering

        // Material applied to this stroke
        glm::vec4 ambientColor;
        glm::vec4 diffuseColor;
        glm::vec4 specularColor;
        float shininess;

        float size;
        DrawStyle style; // Store style used for this stroke
    };

    std::vector<Stroke> strokes;
    std::vector<Stroke> undoneStrokes;
    bool drawing;
    Stroke currentStroke;

    // --- OpenGL Resources ---
    // Generic VBO/VAO for simple styles (lines, points)
    unsigned int simpleVAO, simpleVBO;
    // Resources for instanced rendering (Cubes, Spheres)
    unsigned int instancedVAO, instancedVBO, instanceDataVBO;
    unsigned int cubeEBO; // Use EBO for cube if rendering indexed
    int cubeIndexCount;
    unsigned int sphereVAO, sphereVBO, sphereEBO; // For drawing a single detailed sphere if needed
    int sphereIndexCount;
    // Resources for tube rendering
    unsigned int tubeVAO, tubeVBO, tubeEBO;

    // --- Shaders ---
    unsigned int simpleShaderProgram; // Original shader (renamed)
    unsigned int litShaderProgram;    // New shader with lighting


    DrawStyle currentDrawStyle; // Renamed from drawStyle

    // --- Initialization Helpers ---
    void initShaders();
    void initCube();
    void initSphere(int segments = 16, int rings = 8); // Function to generate sphere mesh
    void initTubeResources(); // Setup VAO/VBO for tubes


    // --- Geometry Generation ---
    void generateTubeMesh(Stroke& stroke, int segments = 8); // Generate vertices/indices for a tube stroke

    // --- Buffer Updates ---
    void updateSimpleBuffer(const std::vector<glm::vec3>& points);
    void updateTubeBuffers(const Stroke& stroke);
    void setupInstancedVertexAttributes(unsigned int VAO, unsigned int& instanceVBO);

    // --- Drawing Helpers ---
    void drawStrokeFreehand(const Stroke& stroke, const glm::mat4& model, int colorLoc);
    void drawStrokePoints(const Stroke& stroke, const glm::mat4& model, int colorLoc);
    void drawStrokeInstanced(const Stroke& stroke, unsigned int baseVAO, int indexCount, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos);

    void smoothStroke(Stroke& stroke);

};