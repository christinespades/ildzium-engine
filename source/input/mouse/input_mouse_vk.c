#include "pch.h"
#include "input_mouse_vk.h"

int mouse_wheel = 0;
float lastX = 640.0f;
float lastY = 360.0f;
int firstMouse = 1; 
double g_mouse_x = 0;
double g_mouse_y = 0;

void handle_mouse_movement(float xoffset, float yoffset)
{
    ActionBinding* action = &g_actions[g_mouse_move_action];

    InputEvent event = {
        .mouse_dx = xoffset,
        .mouse_dy = yoffset
    };

    action->function(&event);
}

int platform_get_mouse_button(int button)
{
    	return glfwGetMouseButton(g_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}

void platform_get_mouse_pos(double* x, double* y)
{
    	glfwGetCursorPos(g_window, x, y);
}

    void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
        g_mouse_x = xpos;
        g_mouse_y = ypos;
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

        handle_mouse_movement(xoffset, yoffset);
    }

    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        mouse_wheel += yoffset;  // accumulate scroll input
        float acceleration = get_param_float(PARAM_CAMERA_ACCELERATION);
        float speed_factor = 1.0f + camera.speed * 0.1f;
        camera.speed += (float)yoffset * acceleration * speed_factor;
        if (camera.speed < 0.5f)   camera.speed = 0.1f;
        if (camera.speed > get_param_float(PARAM_CAMERA_SPEED_MAX)) get_param_float(PARAM_CAMERA_SPEED_MAX);
    }

void set_mouse_callbacks() {
        glfwSetCursorPosCallback(g_window, mouse_callback);
        glfwSetScrollCallback(g_window, scroll_callback);
}