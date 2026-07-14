#pragma once
#include "input/keys/platform_key.h"
#include "input/input_action.h"

// this is reversed since its a axis input/not a key thing
#define INPUT_BIND_MOUSE_MOVE INPUT_ACTION_CAMERA_LOOK

#define INPUT_BIND_ENGINE_EXIT KEY_ESCAPE
#define INPUT_BIND_ENGINE_TOGGLE_MODE KEY_TAB
#define INPUT_BIND_CAMERA_LOOK MOUSE_MOVE
#define INPUT_BIND_CAMERA_MOVE_BACKWARD KEY_S
#define INPUT_BIND_CAMERA_MOVE_DOWN KEY_SPACE
#define INPUT_BIND_CAMERA_MOVE_FORWARD KEY_W
#define INPUT_BIND_CAMERA_MOVE_LEFT KEY_A
#define INPUT_BIND_CAMERA_MOVE_RIGHT KEY_D
#define INPUT_BIND_CAMERA_MOVE_UP KEY_LEFT_SHIFT

#define INPUT_BIND_PARAMS_MAP \
    X(e, INPUT_BIND_MOUSE_MOVE, "Mouse Move", INPUT_ACTION_CAMERA_LOOK, &enum_platform_key, tooltip_EMPTY) \
    X(e, INPUT_BIND_ENGINE_EXIT, "Engine Exit", KEY_ESCAPE, &enum_platform_key, tooltip_EMPTY) \
    X(e, INPUT_BIND_ENGINE_TOGGLE_MODE, "Engine Toggle Mode", KEY_TAB, &enum_platform_key, tooltip_EMPTY) \
    X(e, INPUT_BIND_CAMERA_MOVE_BACKWARD, "Camera Move Backward", KEY_S, &enum_platform_key, tooltip_EMPTY) \
    X(e, INPUT_BIND_CAMERA_MOVE_DOWN, "Camera Move Down", KEY_SPACE, &enum_platform_key, tooltip_EMPTY) \
    X(e, INPUT_BIND_CAMERA_MOVE_FORWARD, "Camera Move Forward", KEY_W, &enum_platform_key, tooltip_EMPTY) \
    X(e, INPUT_BIND_CAMERA_MOVE_LEFT, "Camera Move Left", KEY_A, &enum_platform_key, tooltip_EMPTY) \
    X(e, INPUT_BIND_CAMERA_MOVE_RIGHT, "Camera Move Right", KEY_D, &enum_platform_key, tooltip_EMPTY) \
    X(e, INPUT_BIND_CAMERA_MOVE_UP, "Camera Move Up", KEY_LEFT_SHIFT, &enum_platform_key, tooltip_EMPTY) 

#define INPUT_CAMERA_MIN_PITCH -89.0f
#define INPUT_CAMERA_MAX_PITCH 89.0f
#define INPUT_MOUSE_CURSOR_SENSITIVITY 0.08f
#define INPUT_UI_TUNER_BASE_SPEED      0.00001f
#define INPUT_UI_TUNER_MAX_SPEED       2.0f
#define INPUT_UI_TUNER_ACCEL_TIME      0.3f

#define INPUT_PARAMS_MAP \
    X(f, INPUT_CAMERA_MIN_PITCH, "Camera Min Pitch", -179.0f, 0.0f, tooltip_EMPTY) \
    X(f, INPUT_CAMERA_MAX_PITCH, "Camera Max Pitch", 0.0f, 179.0f, tooltip_EMPTY) \
    X(f, INPUT_MOUSE_CURSOR_SENSITIVITY, "Mouse Cursor Sensitivity", 0.01f, 1.0f, tooltip_PARAM_INPUT_MOUSE_CURSOR_SENSITIVITY) \
    X(f, INPUT_UI_TUNER_ACCEL_TIME, "UI Tuner Acceleration Time", 0.000001f, 10.0f, tooltip_PARAM_INPUT_UI_TUNER_ACCEL_TIME) \
    X(f, INPUT_UI_TUNER_BASE_SPEED, "UI Tuner Speed (Base)", 0.000001f, 10.0f, tooltip_PARAM_INPUT_UI_TUNER_BASE_SPEED) \
    X(f, INPUT_UI_TUNER_MAX_SPEED, "UI Tuner Speed (Max)", 0.000001f, 10.0f, tooltip_PARAM_INPUT_UI_TUNER_MAX_SPEED)
