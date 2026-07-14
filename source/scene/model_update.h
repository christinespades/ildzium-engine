#pragma once
#include "core/params/params.h"
#ifndef __EMSCRIPTEN__
	#include "scene/model_vk_stream_cull.h"
#endif

void update_models(double lastTime);