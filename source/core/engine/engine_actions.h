#pragma once
#ifdef __EMSCRIPTEN__
	#include "engine_actions_webgpu.h"
#else
	#include "engine_actions_vk.h"
#endif