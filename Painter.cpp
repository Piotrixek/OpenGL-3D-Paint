#include "Painter.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Globals.h"
#include <glfw/glfw3.h>
#include <algorithm>
#include <vector>
Painter::Painter() : drawing(false), brushColor(1.0f, 1.0f, 1.0f, 1.0f), brushSize(2.0f), drawStyle(FREEHAND) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    shaderProgram = loadShader("shaders/paint.vert", "shaders/paint.frag");
    initCube();
}
void Painter::initCube() {
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}
void Painter::addPoint(const glm::vec3& point) {
    if (!drawing) {
        drawing = true;
        currentStroke.points.clear();
        currentStroke.color = brushColor;
        currentStroke.size = brushSize;
    }
    currentStroke.points.push_back(point);
}
void Painter::endStroke() {
    if (drawing) {
        if (currentStroke.points.size() > 1) {
            smoothStroke(currentStroke);
            strokes.push_back(currentStroke);
            undoneStrokes.clear();
        }
        drawing = false;
    }
}
void Painter::clear() {
    strokes.clear();
    currentStroke.points.clear();
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
    if (drawing && currentStroke.points.size() > 2) {
        smoothStroke(currentStroke);
    }
}
void Painter::smoothStroke(Stroke& stroke) {
    std::vector<glm::vec3> smoothed;
    smoothed.push_back(stroke.points.front());
    for (size_t i = 1; i < stroke.points.size() - 1; ++i) {
        glm::vec3 prev = stroke.points[i - 1];
        glm::vec3 curr = stroke.points[i];
        glm::vec3 next = stroke.points[i + 1];
        glm::vec3 avg = (prev + curr + next) / 3.0f;
        smoothed.push_back(avg);
    }
    smoothed.push_back(stroke.points.back());
    stroke.points = smoothed;
}
void Painter::updateBuffer(const std::vector<glm::vec3>& points) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
}
void Painter::draw(const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);
    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    int projLoc = glGetUniformLocation(shaderProgram, "projection");
    int colorLoc = glGetUniformLocation(shaderProgram, "strokeColor");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

    glm::mat4 identity = glm::mat4(1.0f);

    if (drawStyle == FREEHAND || drawStyle == POINTS) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &identity[0][0]);
        for (const auto& stroke : strokes) {
            updateBuffer(stroke.points);
            glUniform4fv(colorLoc, 1, glm::value_ptr(stroke.color));
            glLineWidth(stroke.size);
            glBindVertexArray(VAO);
            if (drawStyle == FREEHAND)
                glDrawArrays(GL_LINE_STRIP, 0, stroke.points.size());
            else
                glDrawArrays(GL_POINTS, 0, stroke.points.size());
        }
        if (drawing && currentStroke.points.size() > 1) {
            updateBuffer(currentStroke.points);
            glUniform4fv(colorLoc, 1, glm::value_ptr(currentStroke.color));
            glLineWidth(currentStroke.size);
            glBindVertexArray(VAO);
            if (drawStyle == FREEHAND)
                glDrawArrays(GL_LINE_STRIP, 0, currentStroke.points.size());
            else
                glDrawArrays(GL_POINTS, 0, currentStroke.points.size());
        }
    }
    else if (drawStyle == CUBE) {
        glBindVertexArray(cubeVAO);
        for (const auto& stroke : strokes) {
            glUniform4fv(colorLoc, 1, glm::value_ptr(stroke.color));
            for (const auto& point : stroke.points) {
                glm::mat4 cubeModel = glm::translate(identity, point);
                cubeModel = glm::scale(cubeModel, glm::vec3(stroke.size * 0.1f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &cubeModel[0][0]);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
        if (drawing && currentStroke.points.size() > 1) {
            glUniform4fv(colorLoc, 1, glm::value_ptr(currentStroke.color));
            for (const auto& point : currentStroke.points) {
                glm::mat4 cubeModel = glm::translate(identity, point);
                cubeModel = glm::scale(cubeModel, glm::vec3(currentStroke.size * 0.1f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &cubeModel[0][0]);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
    }
}

void Painter::setBrushColor(const glm::vec4& color) {
    brushColor = color;
    if (!drawing) currentStroke.color = brushColor;
}
void Painter::setBrushSize(float size) {
    brushSize = size;
    if (!drawing) currentStroke.size = brushSize;
}
int Painter::getStrokeCount() const {
    return strokes.size() + (drawing ? 1 : 0);
}
void Painter::removeLastPoint() {
    if (drawing && !currentStroke.points.empty()) {
        currentStroke.points.pop_back();
    }
}
void Painter::scaleCurrentStroke(float scaleFactor) {
    if (drawing && !currentStroke.points.empty()) {
        glm::vec3 center(0.0f);
        for (const auto& p : currentStroke.points)
            center += p;
        center /= currentStroke.points.size();
        for (auto& p : currentStroke.points)
            p = center + (p - center) * scaleFactor;
    }
}
void Painter::translateCurrentStroke(const glm::vec3& translation) {
    if (drawing && !currentStroke.points.empty()) {
        for (auto& p : currentStroke.points)
            p += translation;
    }
}
void Painter::duplicateLastStroke() {
    if (!strokes.empty()) {
        strokes.push_back(strokes.back());
    }
}
void Painter::mergeAllStrokes() {
    if (strokes.empty()) return;
    Stroke merged;
    merged.color = brushColor;
    merged.size = brushSize;
    for (const auto& stroke : strokes) {
        merged.points.insert(merged.points.end(), stroke.points.begin(), stroke.points.end());
    }
    strokes.clear();
    strokes.push_back(merged);
}
void Painter::clearUndoneStrokes() {
    undoneStrokes.clear();
}
void Painter::reverseCurrentStroke() {
    if (drawing && currentStroke.points.size() > 1) {
        std::reverse(currentStroke.points.begin(), currentStroke.points.end());
    }
}
void Painter::setDrawStyle(DrawStyle style) {
    drawStyle = style;
}
Painter::DrawStyle Painter::getDrawStyle() const {
    return drawStyle;
}
