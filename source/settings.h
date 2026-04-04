#pragma once

typedef enum {
    RENDERER_NONE      = 0,
    RENDERER_SHOW_FPS  = 1 << 0,
} RendererFlags;

static uint32_t g_renderer_flags = RENDERER_SHOW_FPS;
extern float g_fps;