#pragma once
#include "rendering/device.h"
#include "scene/model.h"
#include "scene/culling.h"

#ifndef __EMSCRIPTEN__
	void create_instance_buffer(uint32_t capacity);
#endif
void add_model_instance(const char* model_name, const float transform[16], const float color[4]);
void update_model_instances(void);              // upload to GPU (call inside draw_models)
extern uint32_t* g_model_visible_counts;
extern uint32_t* g_model_gpu_offsets;