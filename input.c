#include <GLFW/glfw3.h>
#include "camera.h"
#include "input.h"     // self include for consistency
#include "ui.h"

float lastX = 640.0f;
float lastY = 360.0f;
int firstMouse = 1;    // Use int instead of bool

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = 0;
        return;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.08f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    if (ui_ctx.cursor_captured)
        return;
    
    camera.yaw   += xoffset;
    camera.pitch += yoffset;

    if (camera.pitch > 89.0f)  camera.pitch = 89.0f;
    if (camera.pitch < -89.0f) camera.pitch = -89.0f;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.speed += (float)yoffset * 3.0f;
    if (camera.speed < 1.0f)   camera.speed = 1.0f;
    if (camera.speed > 60.0f)  camera.speed = 60.0f;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        glfwSetInputMode(window,
            GLFW_CURSOR,
            ui_ctx.cursor_captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        ui_ctx.cursor_captured = !ui_ctx.cursor_captured;
    }
}