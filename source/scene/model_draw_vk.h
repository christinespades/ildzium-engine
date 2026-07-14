#pragma once
#include "scene/model_vk.h"

#ifndef __EMSCRIPTEN__
	void draw_models(VkCommandBuffer cmd);
#endif