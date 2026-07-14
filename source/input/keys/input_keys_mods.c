#include "pch.h"
#include "input/keys/input_keys_mods.h"

int platform_get_mods()
{
    int mods = 0;

    if (platform_get_key_down(KEY_LEFT_SHIFT)  || platform_get_key_down(KEY_RIGHT_SHIFT))
        mods |= MOD_SHIFT;

    if (platform_get_key_down(KEY_LEFT_CONTROL) || platform_get_key_down(KEY_RIGHT_CONTROL))
        mods |= MOD_CTRL;

    if (platform_get_key_down(KEY_LEFT_ALT) || platform_get_key_down(KEY_RIGHT_ALT))
        mods |= MOD_ALT;

    return mods;
}