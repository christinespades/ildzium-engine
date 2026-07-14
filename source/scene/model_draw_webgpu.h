#pragma once
#include "scene/model_webgpu.h"

#ifdef __EMSCRIPTEN__
	WGPUBindGroup draw_models_webgpu(WGPURenderPassEncoder pass);
#endif