#include "pch.h"
#include "input_mouse.h"

#ifndef __EMSCRIPTEN__
    float lastX = 640.0f;
    float lastY = 360.0f;
    int firstMouse = 1;    // Use int instead of bool
#else
    static int g_mouse_buttons[3] = {0};
    static double g_mouse_x = 0;
    static double g_mouse_y = 0;
#endif

int platform_get_mouse_button(int button)
{
	#ifdef __EMSCRIPTEN__
	    return g_mouse_buttons[button];
    #else
    	return glfwGetMouseButton(g_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    #endif
}

void platform_get_mouse_pos(double* x, double* y)
{
	#ifdef __EMSCRIPTEN__
	    *x = g_mouse_x;
	    *y = g_mouse_y;
    #else
    	glfwGetCursorPos(g_window, x, y);
    #endif
}

#ifdef __EMSCRIPTEN__
    EM_BOOL mouse_move(int eventType, const EmscriptenMouseEvent* e, void* userData)
    {
        g_mouse_x = e->canvasX;
        g_mouse_y = e->canvasY;
        return 0;
    }

    EM_BOOL mouse_down(int eventType, const EmscriptenMouseEvent* e, void* userData)
    {
        g_mouse_buttons[e->button] = 1;
        return 0;
    }

    EM_BOOL mouse_up(int eventType, const EmscriptenMouseEvent* e, void* userData)
    {
        g_mouse_buttons[e->button] = 0;
        return 0;
    }
#else
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

        if (g_ui_ctx->cursor_captured)
            return;

        camera.yaw   += xoffset;
        camera.pitch -= yoffset;

        if (camera.pitch > 89.0f)  camera.pitch = 89.0f;
        if (camera.pitch < -89.0f) camera.pitch = -89.0f;
    }

    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        mouse_wheel += yoffset;  // accumulate scroll input
        camera.speed += (float)yoffset * 3.0f;
        if (camera.speed < 1.0f)   camera.speed = 1.0f;
        if (camera.speed > 60.0f)  camera.speed = 60.0f;
    }
#endif

void set_mouse_callbacks() {
    #ifdef __EMSCRIPTEN__
        emscripten_set_mousemove_callback("#canvas", 0, 1, mouse_move);
        emscripten_set_mousedown_callback("#canvas", 0, 1, mouse_down);
        emscripten_set_mouseup_callback("#canvas", 0, 1, mouse_up);
    #else
        glfwSetCursorPosCallback(g_window, mouse_callback);
        glfwSetScrollCallback(g_window, scroll_callback);
    #endif
}