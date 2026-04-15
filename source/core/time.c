#include "pch.h"
#include "core/time.h"

#ifdef __EMSCRIPTEN__
	static double g_time = 0.0;
#endif

double ildz_get_time()
{
#ifdef __EMSCRIPTEN__
    return emscripten_get_now() / 1000.0;
#else
    return glfwGetTime();
#endif
}
