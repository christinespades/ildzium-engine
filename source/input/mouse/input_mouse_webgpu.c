#include "pch.h"
#include "input_mouse_webgpu.h"

int mouse_wheel = 0;
float lastX = 640.0f;
float lastY = 360.0f;
int firstMouse = 1; 
int g_mouse_buttons[3] = {0};
double g_mouse_x = 0;
double g_mouse_y = 0;

void handle_mouse_movement(float xoffset, float yoffset) {
    if (g_ui_ctx->cursor_captured)
        return;

    xoffset *= get_param_float(PARAM_INPUT_MOUSE_CURSOR_SENSITIVITY);
    yoffset *= get_param_float(PARAM_INPUT_MOUSE_CURSOR_SENSITIVITY);

    camera.yaw   += xoffset;
    camera.pitch -= yoffset; // Keeping your specific pitch logic

    if (camera.pitch > 89.0f)  camera.pitch = 89.0f;
    if (camera.pitch < -89.0f) camera.pitch = -89.0f;
}

int platform_get_mouse_button(int button)
{
    return g_mouse_buttons[button];
}

void platform_get_mouse_pos(double* x, double* y)
{
    *x = g_mouse_x;
    *y = g_mouse_y;
}

    EM_BOOL mouse_move(int eventType, const EmscriptenMouseEvent* e, void* userData)
    {
        // Update absolute pos for UI/Picking
        g_mouse_x = (double)e->canvasX;
        g_mouse_y = (double)e->canvasY;

        // movementX/Y are the deltas provided by the browser during Pointer Lock
        handle_mouse_movement((float)e->movementX, (float)e->movementY);
        
        return EM_TRUE;
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

    EM_BOOL wheel_callback(int eventType, const EmscriptenWheelEvent* e, void* userData)
    {
        mouse_wheel += (int)(-e->deltaY);   // usually negative for scroll up
        return 0;
    }

    void set_mouse_callbacks() {
        emscripten_set_mousemove_callback("#canvas", 0, 1, mouse_move);
        emscripten_set_mousedown_callback("#canvas", 0, 1, mouse_down);
        emscripten_set_mouseup_callback("#canvas", 0, 1, mouse_up);
        emscripten_set_wheel_callback("#canvas", 0, 1, wheel_callback);
    }