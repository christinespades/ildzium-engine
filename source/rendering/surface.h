#pragma once
#include "input/input.h"

#ifndef __EMSCRIPTEN__
	void create_vk_instance(void);
	void create_vk_surface(void);
#endif