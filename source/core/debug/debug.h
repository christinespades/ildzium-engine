#pragma once
#include "core/debug/debug_log.h"
#ifndef __EMSCRIPTEN__
    #include "core/debug/debug_vk.h"
#endif

// Helper to strip the specific base path prefix
#define STRIP_PREFIX "C:\\Users\\Public\\Downloads\\000github\\ildzium-engine\\source\\"
#define RELEST_PATH(p) (strstr(p, STRIP_PREFIX) ? strstr(p, STRIP_PREFIX) + strlen(STRIP_PREFIX) : p)

// ANSI Escape Codes for Colors
#define ANSI_GREEN "\033[0;32m"
#define ANSI_RED   "\033[0;31m"
#define ANSI_RESET "\033[0m"