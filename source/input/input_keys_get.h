#pragma once
#include "input/input_keys_translate.h"

int platform_get_key_down(platform_key key);
int platform_get_key_up(platform_key key);

#ifdef __EMSCRIPTEN__
    static int g_keys[512] = {0};
#endif