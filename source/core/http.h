#pragma once
#include "core/http_fetch.h"

void platform_http_get(const char* url, void (*callback)(char*, void*), void* user);