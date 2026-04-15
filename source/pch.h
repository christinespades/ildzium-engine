#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <crtdbg.h>
	#include <curl/curl.h>
	#include <dbghelp.h>
	#define GLFW_INCLUDE_VULKAN
	#include <GLFW/glfw3.h>
	#include <vulkan/vulkan.h>
	#include "stb_image.h"
#elif defined(__EMSCRIPTEN__)
    #include <emscripten.h>
	#include <emscripten/html5.h>
	#include <webgpu/webgpu.h>
#endif
#include <ctype.h>    // isalnum
#include <math.h>
#include <stdbool.h>   // for bool type
#include <stdint.h>
#include <stdio.h>    // FILE, fopen, fclose, fread, fseek, ftell, SEEK_END, SEEK_SET
#include <stdlib.h>   // malloc, free
#include <string.h>   // strlen, strcpy, memset
#include <threads.h>        // C11 threads