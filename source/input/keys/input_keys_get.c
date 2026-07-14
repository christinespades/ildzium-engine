#include "pch.h"
#include "input/keys/input_keys_get.h"

unsigned char g_current_keys[512] = {0};
uint8_t g_previous_keys[512] = {0};

int platform_get_key_down(platform_key key)
{
    if (key < 0 || key >= PLATFORM_KEY_COUNT)
        return 0;

    return g_current_keys[key];
}


int platform_get_key_pressed(platform_key key)
{
    if (key < 0 || key >= PLATFORM_KEY_COUNT)
        return 0;

    return g_current_keys[key] && !g_previous_keys[key];
}


int platform_get_key_released(platform_key key)
{
    if (key < 0 || key >= PLATFORM_KEY_COUNT)
        return 0;

    return !g_current_keys[key] && g_previous_keys[key];
}