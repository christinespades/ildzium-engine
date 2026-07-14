#pragma once
#include "input/keys/input_keys_translate.h"

int platform_get_key_down(platform_key key);
int platform_get_key_up(platform_key key);
int platform_get_key_pressed(platform_key key);
int platform_get_key_released(platform_key key);

extern unsigned char g_current_keys[512];
extern uint8_t g_previous_keys[512];