#include "pch.h"
#include "core/http/http_fetch.h"
#include "core/platform.h"
#include "core/string.h"
#ifdef _WIN32
    #define strncasecmp _strnicmp
#endif
#ifndef __EMSCRIPTEN__
	// Helper to look for the ETag header case-insensitively
	size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
	    size_t numbytes = size * nitems;
	    char **etag_store = (char **)userdata;

	    // Check if the header line starts with "etag:" or "ETag:"
	    if (numbytes > 5 && strncasecmp(buffer, "etag:", 5) == 0) {
	        // Locate the start of the actual tag value (skip "etag: " and quotes)
	        char *start = buffer + 5;
	        while (*start == ' ' || *start == '"') start++;
	        
	        // Find the end of the tag value (strip quotes, carriage returns, newlines)
	        char *end = buffer + numbytes - 1;
	        while (end > start && (*end == '\r' || *end == '\n' || *end == '"' || *end == ' ')) {
	            *end = '\0';
	            end--;
	        }

	        // Duplicate and store the string using your custom safe engine function
	        if (*etag_store) free(*etag_store);
	        *etag_store = ildz_strdup(start); 
	    }
	    return numbytes;
	}
	char* fetch_readme(const char* url, const char* old_etag, char** new_etag, long* out_status) {
	    CURL *curl = curl_easy_init();
	    char *result = NULL;
	    struct curl_slist *headers = NULL;

	    if (!curl) return NULL;

	    curl_easy_setopt(curl, CURLOPT_URL, url);
	    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
	    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
	    curl_easy_setopt(curl, CURLOPT_USERAGENT, "ildzium-engine");

	    // Pass old ETag to server if we have it
	    if (old_etag) {
	        char header_buf[512];
	        snprintf(header_buf, sizeof(header_buf), "If-None-Match: \"%s\"", old_etag);
	        headers = curl_slist_append(headers, header_buf);
	        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	    }

	    // Set up header callback to grab the new ETag
	    *new_etag = NULL;
	    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	    curl_easy_setopt(curl, CURLOPT_HEADERDATA, new_etag);

	    CURLcode res = curl_easy_perform(curl);
	    
	    if (res == CURLE_OK) {
	        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, out_status);
	    } else {
	        *out_status = 0; // Request failed structurally
	    }

	    if (headers) curl_slist_free_all(headers);
	    curl_easy_cleanup(curl);

	    return result;
	}
#endif