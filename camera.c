#include "camera.h"
#include <math.h>

Camera camera = {0.0f, 0.0f, 5.0f, -90.0f, 0.0f, 10.0f};

void init_camera(void)
{
    camera.x = 0.0f;
    camera.y = 0.0f;
    camera.z = 5.0f;
    camera.yaw = -90.0f;
    camera.pitch = 0.0f;
    camera.speed = 10.0f;
}

void update_camera(float deltaTime)
{
    float velocity = camera.speed * deltaTime;

    if (glfwGetKey(g_window, GLFW_KEY_W) == GLFW_PRESS) camera.z -= velocity;
    if (glfwGetKey(g_window, GLFW_KEY_S) == GLFW_PRESS) camera.z += velocity;
    if (glfwGetKey(g_window, GLFW_KEY_A) == GLFW_PRESS) camera.x -= velocity;
    if (glfwGetKey(g_window, GLFW_KEY_D) == GLFW_PRESS) camera.x += velocity;
    if (glfwGetKey(g_window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.y += velocity;
    if (glfwGetKey(g_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.y -= velocity;
}