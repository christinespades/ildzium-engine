#include "pch.h"
#include "input_poll.h"

static void input_update_keys(void)
{
    for (uint32_t i = 0; i < PLATFORM_KEY_COUNT; i++)
    {
        g_previous_keys[i] = g_current_keys[i];

#ifdef __EMSCRIPTEN__
        g_current_keys[i] = g_keys[i];
#else
        g_current_keys[i] = glfwGetKey(g_window, i) == GLFW_PRESS;
#endif
    }
}

void input_poll(void)
{
    input_update_keys();

    for (uint32_t i = 0; i < PLATFORM_KEY_COUNT; i++)
    {
        InputActionBinding* input = &g_input_actions[i];

        if (input->action == INPUT_ACTION_NONE)
            continue;

        ActionBinding* action = &g_actions[input->action];

        if (!action->function)
            continue;

        InputEvent event = {
            .key = i
        };

        int trigger_input = 0;

        switch (action->trigger)
        {
            case INPUT_TRIGGER_CONTINUOUS:
                trigger_input = platform_get_key_down((platform_key)i);
                break;

            case INPUT_TRIGGER_ONESHOT:
                trigger_input = platform_get_key_pressed((platform_key)i);
                break;
        }

        if (!trigger_input)
            continue;

        if (g_ui_ctx->cursor_captured &&
            input->action != INPUT_ACTION_ENGINE_EXIT &&
            input->action != INPUT_ACTION_ENGINE_TOGGLE_MODE)
        {
            handle_editor_key((platform_key)i);
            continue;
        }

        action->function(&event);
    }
}