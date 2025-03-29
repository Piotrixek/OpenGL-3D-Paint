#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "Camera.h"
glm::vec3 screenToWorld(GLFWwindow* window, float mouseX, float mouseY, const Camera& camera, float distance);
