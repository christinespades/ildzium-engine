#pragma once
#include "ui/ui_context.h"

typedef struct {
    UI_Context* ctx;
} LoadState;

#ifndef __EMSCRIPTEN__
	char* fetch_readme(const char* url);
#endif