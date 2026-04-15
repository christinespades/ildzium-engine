#ifndef __EMSCRIPTEN__
	#pragma once
	#include "core/window.h"
    #include "rendering/device.h"
    #include "scene/camera.h"
    #include "scene/lights.h"
    #include "scene/model.h"
	#include "scene/model_draw.h"
    #include "scene/sky.h"
    #include "ui/ui.h"
    #include "ui/ui_draw.h"
    #include "ui/ui_renderer.h"

	#define MAX_FRAMES_IN_FLIGHT 3
	void vulkan_draw();
#endif