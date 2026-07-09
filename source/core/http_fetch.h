#pragma once
#include "ui/ui_context.h"

typedef struct {
    UI_Context* ctx;
} LoadState;

#ifndef __EMSCRIPTEN__
	char* fetch_readme(const char* url, const char* old_etag, char** new_etag, long* out_status);
#endif