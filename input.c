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
    static int cursor_captured = 1; // start with cursor disabled

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        if (cursor_captured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            cursor_captured = 0;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            cursor_captured = 1;
        }
        ui_ctx.cursor_visible = cursor_captured; // update UI state
    }
}