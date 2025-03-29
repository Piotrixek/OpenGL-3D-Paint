#include "Util.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
glm::vec3 screenToWorld(GLFWwindow* window, float mouseX, float mouseY, const Camera& camera, float distance) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float x = (2.0f * mouseX) / width - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / height;
    glm::vec4 ray_clip = glm::vec4(x, y, -1.0f, 1.0f);
    glm::mat4 projection = camera.getProjectionMatrix((float)width / height);
    glm::mat4 view = camera.getViewMatrix();
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye.z = -1.0f; ray_eye.w = 0.0f;
    glm::vec3 ray_world = glm::normalize(glm::vec3(glm::inverse(view) * ray_eye));
    glm::vec3 planePoint = camera.position + camera.front * distance;
    glm::vec3 planeNormal = camera.front;
    glm::vec3 rayOrigin = camera.position;
    float t = glm::dot(planePoint - rayOrigin, planeNormal) / glm::dot(ray_world, planeNormal);
    glm::vec3 intersection = rayOrigin + ray_world * t;
    return intersection;
}
