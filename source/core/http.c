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
    // 1. Check if we have fetched this URL before
    CacheEntry* cached = find_in_cache(url);
    const char* old_etag = cached ? cached->etag : NULL;
    
    char* new_etag = NULL;
    long status_code = 0;
    
    // 2. Perform the fetch
    char* network_data = fetch_readme(url, old_etag, &new_etag, &status_code);
    
    if (status_code == 304 && cached) {
        // The server said "304 Not Modified". Use our cached copy!
        // This execution finishes instantly, bypassing heavy network downloads.
        cb(cached->local_data, user);
        
        free(network_data); // network_data is empty anyway
    } 
    else if (status_code == 200 && network_data) {
        // Brand new data or updated data! Save it to cache for next time.
        if (new_etag) {
            save_to_cache(url, new_etag, network_data);
        }
        
        // Pass the fresh data to your engine callback
        cb(network_data, user);
    } 
    else {
        // Handle failure scenarios (429, 404, network drops)
        if (cached) {
            // Fallback to stale cache if rate-limited or offline
            cb(cached->local_data, user); 
        } else {
            cb(NULL, user); // Completely failed
        }
        free(network_data);
    }

    // Clean up the temporary string allocated by the header callback
    if (new_etag) free(new_etag); 
}

#endif