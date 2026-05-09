#pragma once

// Directory iteration
typedef struct PlatformDir PlatformDir;

typedef struct PlatformDirEntry {
    char name[256];
    bool is_directory;
    uint64_t last_modified;   // optional
} PlatformDirEntry;

// Directory functions
PlatformDir* platform_open_dir(const char* path);
bool         platform_read_dir(PlatformDir* dir, PlatformDirEntry* entry);
void         platform_close_dir(PlatformDir* dir);

// File / Folder operations
bool platform_create_directory(const char* path);
bool platform_directory_exists(const char* path);
bool platform_rename(const char* old_path, const char* new_path);
bool platform_delete_directory(const char* path);   // recursive delete recommended

// Time
uint64_t platform_get_file_time(const char* path);  // returns timestamp, higher = newer

// String
char* platform_strdup(const char* str);