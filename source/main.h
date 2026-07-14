#pragma once
#ifdef __EMSCRIPTEN__
	#include "core/engine/engine_webgpu.h"
#else
	#include "core/engine/engine_vk.h"
#endif