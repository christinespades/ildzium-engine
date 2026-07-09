#pragma once
#include "scene/model.h"

#ifdef __EMSCRIPTEN__
	WGPUBindGroup draw_models_webgpu(WGPURenderPassEncoder pass);
#else
    void draw_models(VkCommandBuffer cmd);
#endif