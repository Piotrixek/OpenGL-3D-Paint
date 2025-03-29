#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "glad/glad.h"
#include <glm/gtc/type_ptr.hpp>
#include <string>
class Painter {
public:
    enum DrawStyle { FREEHAND, CUBE, POINTS };
    Painter();
    void addPoint(const glm::vec3& point);
    void endStroke();
    void clear();
    void draw(const glm::mat4& view, const glm::mat4& projection);
    void undoStroke();
    void redoStroke();
    void smoothCurrentStroke();
    void setBrushColor(const glm::vec4& color);
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
    glm::vec4 brushColor;
    float brushSize;
private:
    struct Stroke {
        std::vector<glm::vec3> points;
        glm::vec4 color;
        float size;
    };
    std::vector<Stroke> strokes;
    std::vector<Stroke> undoneStrokes;
    bool drawing;
    Stroke currentStroke;
    unsigned int VAO, VBO;
    unsigned int cubeVAO, cubeVBO;
    unsigned int shaderProgram;
    DrawStyle drawStyle;
    void updateBuffer(const std::vector<glm::vec3>& points);
    void smoothStroke(Stroke& stroke);
    void initCube();
};
