#include "pch.h"
#include "input/input_action.h"

ActionBinding g_actions[INPUT_ACTION_COUNT];
InputActionBinding g_input_actions[512] = {0};
InputAction g_mouse_move_action;

#define INPUT_ACTION_BINDINGS(X) \
    X(CAMERA_MOVE_LEFT,     camera_move_left,     CONTINUOUS, 1) \
    X(CAMERA_MOVE_RIGHT,    camera_move_right,    CONTINUOUS, 1) \
    X(CAMERA_MOVE_BACKWARD, camera_move_backward, CONTINUOUS, 1) \
    X(CAMERA_MOVE_FORWARD,  camera_move_forward,  CONTINUOUS, 1) \
    X(CAMERA_MOVE_UP,       camera_move_up,       CONTINUOUS, 1) \
    X(CAMERA_MOVE_DOWN,     camera_move_down,     CONTINUOUS, 1) \
    X(CAMERA_LOOK,          camera_look,          CONTINUOUS, 0) \
    X(ENGINE_EXIT,          engine_exit,          ONESHOT,    1) \
    X(ENGINE_TOGGLE_MODE,   engine_toggle_mode,   ONESHOT,    1)
        
void init_input_actions(void)
{

    #define X(name, fn, trigger, bind) \
        g_actions[INPUT_ACTION_##name] = (ActionBinding){ \
            fn, \
            INPUT_TRIGGER_##trigger \
        };

        INPUT_ACTION_BINDINGS(X)

    #undef X

    // we do this trick to ignore the ones with bind 0
    // since for them the map scheme is reversed;
    // mouse move to camera_look vs the opposite

    #define INPUT_BINDING_1(name) \
        g_input_actions[get_param_enum(PARAM_INPUT_BIND_##name)] = (InputActionBinding){ \
            INPUT_ACTION_##name \
        };

    #define INPUT_BINDING_0(name)

    #define INPUT_BINDING_SELECT(x) INPUT_BINDING_##x

    #define X(name, fn, trigger, bind) \
        INPUT_BINDING_SELECT(bind)(name)

    INPUT_ACTION_BINDINGS(X)

    #undef X
    #undef INPUT_BINDING_SELECT
    #undef INPUT_BINDING_1
    #undef INPUT_BINDING_0

    g_mouse_move_action = get_param_enum(PARAM_INPUT_BIND_MOUSE_MOVE);
}