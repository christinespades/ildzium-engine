#include "pch.h"
#include "core/http.h"

#ifdef __EMSCRIPTEN__
	EM_JS(void, js_http_get, (const char* url, void (*cb)(char*, void*), void* user), {
	    const str = UTF8ToString(url);

	    fetch(str)
	        .then(r => r.text())
	        .then(text => {
	            const len = lengthBytesUTF8(text) + 1;
	            const buf = _malloc(len);
	            stringToUTF8(text, buf, len);

	            dynCall_vii(cb, buf, user);
	            _free(buf);
	        })
	        .catch(() => {
	            dynCall_vii(cb, 0, user);
	        });
	});
#endif

void platform_http_get(const char* url, void (*callback)(char*, void*), void* user)
{
#ifdef __EMSCRIPTEN__
    js_http_get(url, callback, user);
#else
    char* data = fetch_readme(url);
    callback(data, user);
#endif
}