#include "pch.h"
#include "camera_actions.h"

void camera_look(const InputEvent *event)
{
    if (g_ui_ctx->cursor_captured)
        return;
    float xoffset = event->mouse_dx;
    float yoffset = event->mouse_dy;

    xoffset *= get_param_float(PARAM_INPUT_MOUSE_CURSOR_SENSITIVITY);
    yoffset *= get_param_float(PARAM_INPUT_MOUSE_CURSOR_SENSITIVITY);
    float maxPitch = get_param_float(PARAM_INPUT_CAMERA_MAX_PITCH);
    float minPitch = get_param_float(PARAM_INPUT_CAMERA_MIN_PITCH);

    camera.yaw += xoffset;
    camera.pitch -= yoffset;

    if (camera.pitch > maxPitch)
        camera.pitch = maxPitch;

    if (camera.pitch < minPitch)
        camera.pitch = minPitch;
}

void camera_move_backward(const InputEvent *event)
{
    camera.x -= camera.forward.x * camera.velocity;
    camera.z -= camera.forward.z * camera.velocity;
}

void camera_move_down(const InputEvent *event)
{
    camera.y += camera.velocity;
}

void camera_move_forward(const InputEvent *event)
{
    camera.x += camera.forward.x * camera.velocity;
    camera.z += camera.forward.z * camera.velocity;
}

void camera_move_left(const InputEvent *event)
{
    camera.x -= camera.right.x * camera.velocity;
    camera.z -= camera.right.z * camera.velocity;
}

void camera_move_right(const InputEvent *event)
{
    camera.x += camera.right.x * camera.velocity;
    camera.z += camera.right.z * camera.velocity;
}

void camera_move_up(const InputEvent *event)
{
    camera.y -= camera.velocity;
}