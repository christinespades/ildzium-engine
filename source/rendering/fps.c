#include "pch.h"
#include "rendering/fps.h"

uint32_t g_current_frame = 0;
double g_dt = 0.0f;
uint32_t* g_ui_fb = NULL;
int g_target_fps;
double g_last_render_time;
double g_frame_duration;
float g_fps = 0.0f;