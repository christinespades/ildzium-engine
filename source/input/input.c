#include <GLFW/glfw3.h>
#include "core/io.h"
#include "scene/camera.h"
#include "input.h"
#include "ui/ui.h"
#include "ui/ui_editor.h"

float lastX = 640.0f;
float lastY = 360.0f;
int firstMouse = 1;    // Use int instead of bool

extern UI_Context* g_ui_ctx;

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
    if (!g_ui_ctx->cursor_captured) return;

    // Find focused editable button (for now assume only one, or add focus logic)
    for (int i = 0; i < g_ui_ctx->button_count; i++) {
        UI_Button* b = &g_ui_ctx->buttons[i];
        if (b->is_editable) {
            // Insert character at cursor
            insert_char_at_cursor(b, (char)codepoint);
            return;
        }
    }
}

void handle_editor_keys(int key, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    for (int i = 0; i < g_ui_ctx->button_count; i++) {
        UI_Button* b = &g_ui_ctx->buttons[i];
        if (!b->is_editable) continue;

        bool shift = (mods & GLFW_MOD_SHIFT);

        if (key == GLFW_KEY_BACKSPACE) {
            delete_char_before_cursor(b);
        }
        else if (key == GLFW_KEY_DELETE) {
            delete_char_at_cursor(b);
        }
        else if (key == GLFW_KEY_LEFT) {
            int steps = (mods & GLFW_MOD_CONTROL) ? 5 : 1; // rough "word" step
            move_cursor_left(b, steps);
            if (shift) {
                if (b->selection_start == -1) b->selection_start = b->cursor_pos + steps;
                b->selection_end = b->cursor_pos;
            } else {
                b->selection_start = b->selection_end = -1;
            }
        }
        else if (key == GLFW_KEY_RIGHT) {
            int steps = (mods & GLFW_MOD_CONTROL) ? 5 : 1;
            move_cursor_right(b, steps);
            if (shift) {
                if (b->selection_start == -1) b->selection_start = b->cursor_pos - steps;
                b->selection_end = b->cursor_pos;
            } else {
                b->selection_start = b->selection_end = -1;
            }
        }
        else if (key == GLFW_KEY_HOME) {
            move_to_home(b);
            if (!shift) b->selection_start = b->selection_end = -1;
            else if (b->selection_start == -1) b->selection_start = b->cursor_pos; // simplistic
        }
        else if (key == GLFW_KEY_END) {
            move_to_end(b);
            if (!shift) b->selection_start = b->selection_end = -1;
        }
        else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) {
            insert_char_at_cursor(b, '\n');
        }
        else if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_S) {
            save_file(b->filepath, b->content);
        }
        else if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_C) {
            copy_selection_to_clipboard(b);
        }
        else if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_V) {
            paste_from_clipboard(b);
        }
        else if (key == GLFW_KEY_TAB) return;
        if (key == GLFW_KEY_ESCAPE) return;
    }
}

// Also enhance key_callback for backspace, delete, arrows, ctrl+c/v/s, etc.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (!g_ui_ctx) return;
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }

    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        g_ui_ctx->cursor_captured = !g_ui_ctx->cursor_captured;

        glfwSetInputMode(window,
            GLFW_CURSOR,
            g_ui_ctx->cursor_captured ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

    // Route editing keys when UI is captured
    if (g_ui_ctx->cursor_captured) {
        handle_editor_keys(key, action, mods);
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

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

void init_input()
{
    glfwSetKeyCallback(g_window, key_callback);
    glfwSetCharCallback(g_window, character_callback);
    glfwSetCursorPosCallback(g_window, mouse_callback);
    glfwSetScrollCallback(g_window, scroll_callback);
    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}