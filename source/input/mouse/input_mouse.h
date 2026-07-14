#pragma once
#ifdef __EMSCRIPTEN__
    #include "input_mouse_webgpu.h"
#else
    #include "input_mouse_vk.h"
#endif