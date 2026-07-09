#pragma once
#include "core/http_cache.h"
#include "core/http_fetch.h"

#ifdef __EMSCRIPTEN__
void js_http_get(const char* url, void* pack);
void js_dispatch_http_result(char* data, int id);
void invoke_http_callback(char* data, void* cb, void* user);
#endif

typedef void (*http_callback)(char*, void*);

void platform_http_get(const char* url, http_callback cb, void* user);