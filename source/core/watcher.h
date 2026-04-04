#pragma once
#include <threads.h>        // C11 threads
#include <stdbool.h>
#include <windows.h>

// Max number of directories we can watch simultaneously
#define MAX_WATCHED_DIRS 16
#define MAX_RELOAD_QUEUE 64

// Callback type that the renderer will provide
typedef void (*ShaderReloadCallback)(const char* shader_path);  // full path to .vert / .frag

typedef struct {
    char* dir_path;           // e.g. "../../shaders"
    char** extensions;        // e.g. {".vert", ".frag", NULL}
    int   num_extensions;
} WatchConfig;

typedef struct {
    HANDLE hDir;
    OVERLAPPED overlapped;
    BYTE buffer[16 * 1024];   // Should be enough for most changes
    WatchConfig config;
    bool active;
} DirectoryWatcher;

typedef struct {
    char shader_path[MAX_PATH];
} ReloadEntry;

// Initialize the watcher system (call once at startup)
bool watcher_init(void);
bool watcher_add_directory(const char* dir_path, const char* extensions[], int num_extensions);
void watcher_set_glslc_path(const char* path);

// Add a directory to watch
bool watcher_add_directory(const char* dir_path, const char* extensions[], int num_extensions);

// Process pending file change events (non-blocking)
// Call this every frame or in a background thread
void watcher_update(void);

void watcher_cleanup(void);