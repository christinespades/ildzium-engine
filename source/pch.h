#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#include <ctype.h>    // isalnum
#include <curl/curl.h>
#include <dbghelp.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdbool.h>   // for bool type
#include <stdint.h>
#include <stdio.h>    // FILE, fopen, fclose, fread, fseek, ftell, SEEK_END, SEEK_SET
#include <stdlib.h>   // malloc, free
#include <string.h>   // strlen, strcpy, memset
#include <threads.h>        // C11 threads
#include <vulkan/vulkan.h>
