#pragma once

typedef enum {
    CAMERA_NONE          = 0,
    CAMERA_SHOW_POS    = 1 << 0,
    CAMERA_SHOW_SPEED  = 1 << 1,
    CAMERA_SHOW_YAW_PITCH  = 1 << 2,
} CameraFlags;

static uint32_t g_camera_flags =
    CAMERA_SHOW_POS |
    CAMERA_SHOW_SPEED |
    CAMERA_SHOW_YAW_PITCH;

typedef enum {
    RENDERER_NONE      = 0,
    RENDERER_SHOW_FPS  = 1 << 0,
    RENDERER_SHOW_DRAWN    = 1 << 1,
    RENDERER_SHOW_CULLED   = 1 << 2,
} RendererFlags;

static uint32_t g_renderer_flags =
    RENDERER_SHOW_FPS |
    RENDERER_SHOW_DRAWN |
    RENDERER_SHOW_CULLED;
extern float g_fps;
extern uint32_t g_drawn_count;
extern uint32_t g_culled_count;

// toggle_flag(&g_camera_flags, CAMERA_SHOW_POS);
static inline void toggle_flag(uint32_t *flags, uint32_t flag)
{
    *flags ^= flag;
}