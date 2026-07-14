#include "pch.h"
#include "http_cache.h"

// Internal cache state
static CacheEntry* g_cache = NULL;
static int g_cache_count = 0;
static int g_cache_capacity = 0;

// A simple, fast string hashing algorithm (DJB2) 
// to map a URL string to a unique unsigned long
unsigned long hash_url(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

// Helper to ensure the .cache directory exists using your platform abstraction
void ensure_cache_dir_exists() {
    platform_create_directory(".cache");
}

CacheEntry* find_in_cache(const char* url) {
    if (!url) return NULL;

    ensure_cache_dir_exists();
    unsigned long hash = hash_url(url);
    
    char txt_path[64];
    char etag_path[64];
    snprintf(txt_path, sizeof(txt_path), ".cache/%08lx.txt", hash);
    snprintf(etag_path, sizeof(etag_path), ".cache/%08lx.etag", hash);

    // Try to open the files
    FILE* txt_file = fopen(txt_path, "r");
    FILE* etag_file = fopen(etag_path, "r");

    if (!txt_file || !etag_file) {
        if (txt_file) fclose(txt_file);
        if (etag_file) fclose(etag_file);
        return NULL; // Cache miss (one or both files missing)
    }

    // Allocate a static or persistent structure to pass back to the platform layer
    static CacheEntry shared_entry = {0};
    
    // Free previous entry strings if they exist to prevent memory leaks
    if (shared_entry.url) free(shared_entry.url);
    if (shared_entry.etag) free(shared_entry.etag);
    if (shared_entry.local_data) free(shared_entry.local_data);

    shared_entry.url = ildz_strdup(url);

    // Read ETag
    fseek(etag_file, 0, SEEK_END);
    long etag_size = ftell(etag_file);
    fseek(etag_file, 0, SEEK_SET);
    shared_entry.etag = malloc(etag_size + 1);
    if (shared_entry.etag) {
        size_t read_bytes = fread(shared_entry.etag, 1, etag_size, etag_file);
        shared_entry.etag[read_bytes] = '\0';
    }

    // Read Local Data
    fseek(txt_file, 0, SEEK_END);
    long txt_size = ftell(txt_file);
    fseek(txt_file, 0, SEEK_SET);
    shared_entry.local_data = malloc(txt_size + 1);
    if (shared_entry.local_data) {
        size_t read_bytes = fread(shared_entry.local_data, 1, txt_size, txt_file);
        shared_entry.local_data[read_bytes] = '\0';
    }

    fclose(txt_file);
    fclose(etag_file);

    return &shared_entry;
}

void save_to_cache(const char* url, const char* etag, const char* data) {
    if (!url || !data) return;

    ensure_cache_dir_exists();
    unsigned long hash = hash_url(url);

    char txt_path[64];
    char etag_path[64];
    snprintf(txt_path, sizeof(txt_path), ".cache/%08lx.txt", hash);
    snprintf(etag_path, sizeof(etag_path), ".cache/%08lx.etag", hash);

    // Save content data
    FILE* txt_file = fopen(txt_path, "w");
    if (txt_file) {
        fputs(data, txt_file);
        fclose(txt_file);
    }

    // Save ETag companion data (if available)
    if (etag) {
        FILE* etag_file = fopen(etag_path, "w");
        if (etag_file) {
            fputs(etag, etag_file);
            fclose(etag_file);
        }
    }
}