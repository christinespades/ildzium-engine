#include "pch.h"
#include "input/input_keys_get.h"

unsigned char g_keys[512] = {0};

int platform_get_key_down(platform_key key)
{
#ifdef __EMSCRIPTEN__
    return (key >= 0 && key < 512) ? g_keys[key] : 0;
#else
    return glfwGetKey(g_window, key) == GLFW_PRESS;
#endif
}

int platform_get_key_up(platform_key key)
{
#ifdef __EMSCRIPTEN__
    return (key >= 0 && key < 512) ? !g_keys[key] : 1;
#else
    return glfwGetKey(g_window, key) == GLFW_RELEASE;
#endif
}