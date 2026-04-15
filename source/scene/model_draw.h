#pragma once
#include "scene/model.h"

#ifdef __EMSCRIPTEN__
	void draw_models_webgpu(WGPURenderPassEncoder pass);
#else
    void draw_models(VkCommandBuffer cmd);          // one draw call for ALL instances
#endif