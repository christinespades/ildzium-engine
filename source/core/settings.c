#include "pch.h"
#include "core/settings.h"

uint32_t g_drawn_count = 0;
uint32_t g_culled_count = 0;
bool g_enable_distance_culling = true;
bool g_enable_frustum_culling  = true;
uint32_t g_stats_distance_culled = 0;
uint32_t g_stats_frustum_culled  = 0;