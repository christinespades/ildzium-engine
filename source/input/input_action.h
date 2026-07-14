#pragma once
#include "core/engine/engine_actions.h"
#include "core/params/params.h"
#include "scene/camera_actions.h"
#include "input/keys/platform_key.h"

typedef enum
{
	INPUT_ACTION_NONE = 0,
    INPUT_ACTION_CAMERA_MOVE_LEFT,
    INPUT_ACTION_CAMERA_MOVE_RIGHT,
    INPUT_ACTION_CAMERA_MOVE_BACKWARD,
    INPUT_ACTION_CAMERA_MOVE_FORWARD,
    INPUT_ACTION_CAMERA_MOVE_UP,
    INPUT_ACTION_CAMERA_MOVE_DOWN,
    INPUT_ACTION_CAMERA_LOOK,
    INPUT_ACTION_ENGINE_EXIT,
    INPUT_ACTION_ENGINE_TOGGLE_MODE,

    INPUT_ACTION_COUNT
} InputAction;

typedef struct
{
    InputAction action;
} InputActionBinding;

InputActionBinding g_input_actions[PLATFORM_KEY_COUNT];

typedef enum
{
    INPUT_TRIGGER_CONTINUOUS,
    INPUT_TRIGGER_ONESHOT
} InputTrigger;

typedef void (*ActionFn)(const InputEvent *event);

typedef struct
{
    ActionFn function;
    InputTrigger trigger;
} ActionBinding;

extern ActionBinding g_actions[INPUT_ACTION_COUNT];
extern InputAction g_mouse_move_action;

void init_input_actions(void);