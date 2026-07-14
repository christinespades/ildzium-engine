#pragma once
#include "core/params/params.h"
#include "input/input_event.h"
#include "ui/ui_context.h"
#include "scene/camera.h"

void camera_look(const InputEvent *event);
void camera_move_down(const InputEvent *event);
void camera_move_up(const InputEvent *event);
void camera_move_left(const InputEvent *event);
void camera_move_right(const InputEvent *event);
void camera_move_forward(const InputEvent *event);
void camera_move_backward(const InputEvent *event);