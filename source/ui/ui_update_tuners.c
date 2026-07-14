#include "pch.h"
#include "ui/ui_update_tuners.h"

void ui_update_tuners(UI_Button* b, int mouse_pressed, int mouse_pressed_this_frame)
{
    if (b->target_value == NULL)
        return;

    if (!mouse_pressed)
    {
        b->hold_time = 0.0f;
        b->last_direction = 0;
        return;
    }

    float current_value = 0.0f;

    // Read current value as float for the slider positioning logic
    switch (b->type)
    {
        case PARAM_TYPE_FLOAT:
            current_value = *(float*)b->target_value;
            break;
        case PARAM_TYPE_ENUM:
            current_value = (float)*(int*)b->target_value;
            break;
        default:
            return;
    }


    // Calculate the dynamic split line position based on current value
    float range = b->max_value - b->min_value;
    int mid = b->x + b->w / 2;

    int offset = 0;

    if (range > 0.0f)
    {
        offset = (int)(((current_value - b->min_value) * b->w) / range)
            - b->w / 2;
    }

    int dynamic_split_x = mid + offset;

    // Determine direction
    int current_direction = (g_mouse_x < dynamic_split_x) ? -1 : 1;

    // Reset acceleration on new press or direction change
    if (mouse_pressed_this_frame ||
        (b->last_direction != 0 && b->last_direction != current_direction))
    {
        b->hold_time = 0.0f;
    }

    b->last_direction = current_direction;

    b->hold_time += g_dt;


    float t = b->hold_time / get_param_float(PARAM_INPUT_UI_TUNER_ACCEL_TIME);

    if (t > 1.0f)
        t = 1.0f;


    float speed =
        get_param_float(PARAM_INPUT_UI_TUNER_BASE_SPEED) +
        (get_param_float(PARAM_INPUT_UI_TUNER_MAX_SPEED) -
         get_param_float(PARAM_INPUT_UI_TUNER_BASE_SPEED)) * (t * t);


    float step;

    if (b->step_size > 0.0f)
        step = b->step_size;
    else
        step = range * speed;


    float delta = step * speed * g_dt;


    // Apply value change depending on type
    switch (b->type)
    {
        case PARAM_TYPE_FLOAT:
        {
            float* value = (float*)b->target_value;

            if (current_direction == -1)
                *value -= delta;
            else
                *value += delta;

            if (*value < b->min_value)
                *value = b->min_value;

            if (*value > b->max_value)
                *value = b->max_value;

            break;
        }


        case PARAM_TYPE_ENUM:
        {
            int* value = (int*)b->target_value;

            // enums should move in whole steps
            if (mouse_pressed_this_frame)
            {
                if (current_direction == -1)
                    (*value)--;
                else
                    (*value)++;
            }

            if (*value < (int)b->min_value)
                *value = (int)b->min_value;

            if (*value > (int)b->max_value)
                *value = (int)b->max_value;

            break;
        }


        default:
            break;
    }
}
