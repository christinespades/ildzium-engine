#pragma once

typedef struct {
    char* url;
    char* etag;
    char* local_data;
} CacheEntry;

// A simple, fast string hashing algorithm (DJB2) 
// to map a URL string to a unique unsigned long
unsigned long hash_url(const char *str);
// Helper to ensure the .cache directory exists
void ensure_cache_dir_exists();
CacheEntry* find_in_cache(const char* url);
void save_to_cache(const char* url, const char* etag, const char* data);