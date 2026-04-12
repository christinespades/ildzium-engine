#pragma once
#include "core/window.h"

void surface_init(void);

#ifndef __EMSCRIPTEN__
	void init_glfw(void);
	void create_vulkan_instance(void);
	void create_surface(void);
#endif