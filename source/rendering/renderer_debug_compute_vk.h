#pragma once
#include "rendering/renderer_debug.h"
#include "rendering/device.h"
#include "rendering/shaders_vk.h"

void create_debug_compute_pipeline();
void renderer_debug_update_compute_descriptors(uint32_t currentFrame);