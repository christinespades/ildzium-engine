#include "pch.h"
#include "core/http.h"

#ifdef __EMSCRIPTEN__

EMSCRIPTEN_KEEPALIVE
void invoke_http_callback(char* data, void* cb, void* user)
{
    ((http_callback)cb)(data, user);
}

void platform_http_get(const char* url, http_callback cb, void* user)
{
    void* pack[2] = { (void*)cb, user };
    js_http_get(url, pack);
}

EMSCRIPTEN_KEEPALIVE
void on_http_result(char* data, int id)
{
    js_dispatch_http_result(data, id);
}

#else

void platform_http_get(const char* url, http_callback cb, void* user)
{
    char* data = fetch_readme(url);
    cb(data, user);
}

#endif