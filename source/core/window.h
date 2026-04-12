#pragma once

#ifdef __EMSCRIPTEN__
    static int g_width = 1280;
    static int g_height = 720;
#else
    GLFWwindow* g_window;
#endif