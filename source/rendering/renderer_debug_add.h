#pragma once
#include "rendering/renderer_debug.h"
#include "scene/camera.h"

void debug_add_line(
    float sx,float sy,float sz,
    float ex,float ey,float ez,
    float width,
    float r,float g,float b);
void debug_add_grid(float width);
void renderer_debug_add_primitives();