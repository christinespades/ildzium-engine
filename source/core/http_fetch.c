#include "pch.h"
#include "core/http_fetch.h"
#include "core/string.h"

#ifndef __EMSCRIPTEN__
	// Fetch a URL into a malloc'd string (caller must free)
	char* fetch_readme(const char* url) {
	    CURL *curl = curl_easy_init();
	    char *result = NULL;
	    if (curl) {
	        curl_easy_setopt(curl, CURLOPT_URL, url);
	        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
	        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
	        curl_easy_setopt(curl, CURLOPT_USERAGENT, "ildzium-engine"); // GitHub requires User-Agent
	        curl_easy_perform(curl);
	        curl_easy_cleanup(curl);
	    }
	    return result;
	}
#endif