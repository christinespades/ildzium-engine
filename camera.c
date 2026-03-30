#include "camera.h"
#include <math.h>

Camera camera = {0.0f, 0.0f, 5.0f, -90.0f, 0.0f, 15.0f};

void init_camera(void)
{
    camera.x     = 0.0f;
    camera.y     = 0.0f;
    camera.z     = 5.0f;
    camera.yaw   = -90.0f;
    camera.pitch = 0.0f;
    camera.speed = 15.0f;
}

void update_camera(float deltaTime)
{
    float velocity = camera.speed * deltaTime;

    // Convert yaw to radians for sin/cos
    float yawRad = camera.yaw * 0.0174532925f;     // PI/180 ≈ 0.0174532925

    // Forward / Backward (W/S) - relative to yaw
    if (glfwGetKey(g_window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.x += sinf(yawRad) * velocity;
        camera.z += cosf(yawRad) * velocity;
    }
    if (glfwGetKey(g_window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.x -= sinf(yawRad) * velocity;
        camera.z -= cosf(yawRad) * velocity;
    }

    // Strafe Left / Right (A/D)
    float strafeYaw = yawRad - 1.57079632679f;     // yaw - 90 degrees
    if (glfwGetKey(g_window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.x += sinf(strafeYaw) * velocity;
        camera.z += cosf(strafeYaw) * velocity;
    }
    if (glfwGetKey(g_window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.x -= sinf(strafeYaw) * velocity;
        camera.z -= cosf(strafeYaw) * velocity;
    }

    // Up / Down (Space / Shift)
    if (glfwGetKey(g_window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.y += velocity;

    if (glfwGetKey(g_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.y -= velocity;
}