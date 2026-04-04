#include "watcher.h"
#include "rendering/shaders.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void on_shader_changed(const char* path);

static DirectoryWatcher g_watchers[MAX_WATCHED_DIRS] = {0};
static int g_num_watchers = 0;
static thrd_t g_watcher_thread;
static volatile LONG g_running = 0;

static ShaderReloadCallback g_reload_callback = NULL;

// Simple lock-free queue for reload requests (main thread consumes)
static ReloadEntry g_reload_queue[MAX_RELOAD_QUEUE];
static volatile LONG g_queue_head = 0;
static volatile LONG g_queue_tail = 0;

static char g_glslc_path[MAX_PATH] = "glslc.exe";

void init_glslc_path(void)
{
    const char* sdk = getenv("VULKAN_SDK");
    if (sdk) {
        snprintf(g_glslc_path, sizeof(g_glslc_path),
                 "%s\\Bin\\glslc.exe", sdk);
    } else {
        printf("VULKAN_SDK not set, using fallback: %s\n", g_glslc_path);
    }
}

static bool ends_with(const char* str, const char* suffix)
{
    if (!str || !suffix) return false;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr) return false;
    return _stricmp(str + lenstr - lensuffix, suffix) == 0;
}

void compile_shader(const char* full_path)
{
    char cmdline[4096];
    snprintf(cmdline, sizeof(cmdline), "\"%s\" \"%s\" -o \"%s.spv\"",
             g_glslc_path, full_path, full_path);

    STARTUPINFOA si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi;
    if (!CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        printf("[Watcher] Failed to launch glslc.exe\n");
        return;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exit_code == 0) {
        printf("[Watcher] Success\n");

        // Enqueue shader for hot reload
        int tail = g_queue_tail;
        int next = (tail + 1) % MAX_RELOAD_QUEUE;
        if (next != g_queue_head) {
            strcpy(g_reload_queue[tail].shader_path, full_path);
            InterlockedExchange(&g_queue_tail, next);
        } else {
            printf("[Watcher] Reload queue full!\n");
        }
    } else {
        printf("[Watcher] Compilation failed (code %lu)\n", exit_code);
    }
}

static void process_changes(DirectoryWatcher* w)
{
    DWORD bytesReturned = 0;
    if (!GetOverlappedResult(w->hDir, &w->overlapped, &bytesReturned, FALSE))
        return;

    if (bytesReturned == 0) return;

    FILE_NOTIFY_INFORMATION* notify = (FILE_NOTIFY_INFORMATION*)w->buffer;
    char filename[MAX_PATH];

    while (true) {
        int len = WideCharToMultiByte(CP_UTF8, 0, notify->FileName,
                                      notify->FileNameLength / sizeof(WCHAR),
                                      filename, sizeof(filename)-1, NULL, NULL);
        filename[len] = '\0';

        if (notify->Action == FILE_ACTION_MODIFIED || notify->Action == FILE_ACTION_ADDED) {
            for (int i = 0; i < w->config.num_extensions; ++i) {
                if (ends_with(filename, w->config.extensions[i])) {
                    char abs_path[MAX_PATH];
                    snprintf(abs_path, sizeof(abs_path), "%s\\%s", w->config.dir_path, filename);
                    compile_shader(abs_path);
                    break;
                }
            }
        }

        if (notify->NextEntryOffset == 0) break;
        notify = (FILE_NOTIFY_INFORMATION*)((BYTE*)notify + notify->NextEntryOffset);
    }

    // Re-arm
    memset(&w->overlapped, 0, sizeof(OVERLAPPED));
    ReadDirectoryChangesW(w->hDir, w->buffer, sizeof(w->buffer), TRUE,
                          FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
                          NULL, &w->overlapped, NULL);
}

static int watcher_thread_func(void* arg)
{
    printf("[Watcher] Background thread started\n");

    while (g_running) {
        bool had_event = false;

        for (int i = 0; i < g_num_watchers; ++i) {
            DirectoryWatcher* w = &g_watchers[i];
            if (!w->active) continue;

            if (HasOverlappedIoCompleted(&w->overlapped)) {
                process_changes(w);
                had_event = true;
            }
        }

        // Sleep a bit if nothing happened (reduces CPU usage)
        if (!had_event) {
            Sleep(8);   // ~120 Hz polling when idle — feel free to adjust
        }
    }

    printf("[Watcher] Background thread shutting down\n");
    return 0;
}

bool watcher_init()
{
    init_glslc_path();
    const char* shader_exts[] = { ".vert", ".frag", ".comp" };  // add more if needed
    watcher_add_directory("../../shaders", shader_exts, 3);

    g_reload_callback = on_shader_changed;
    InterlockedExchange(&g_running, 1);
    InterlockedExchange(&g_queue_head, 0);
    InterlockedExchange(&g_queue_tail, 0);

    if (thrd_create(&g_watcher_thread, watcher_thread_func, NULL) != thrd_success) {
        printf("Failed to create watcher thread\n");
        return false;
    }
    return true;
}

bool watcher_add_directory(const char* dir_path, const char* extensions[], int num_extensions)
{
    if (g_num_watchers >= MAX_WATCHED_DIRS) {
        printf("Too many watched directories!\n");
        return false;
    }

    DirectoryWatcher* w = &g_watchers[g_num_watchers];

    // Open directory handle
    w->hDir = CreateFileA(
        dir_path,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (w->hDir == INVALID_HANDLE_VALUE) {
        printf("Failed to open directory for watching: %s\n", dir_path);
        return false;
    }

    // Store config (we own the strings now)
    w->config.dir_path = _strdup(dir_path);
    w->config.extensions = malloc(num_extensions * sizeof(char*));
    w->config.num_extensions = num_extensions;

    for (int i = 0; i < num_extensions; ++i) {
        w->config.extensions[i] = _strdup(extensions[i]);
    }

    // Start watching
    memset(&w->overlapped, 0, sizeof(OVERLAPPED));
    BOOL success = ReadDirectoryChangesW(
        w->hDir,
        w->buffer,
        sizeof(w->buffer),
        TRUE,   // recursive
        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
        NULL,
        &w->overlapped,
        NULL
    );

    if (!success) {
        printf("ReadDirectoryChangesW failed for %s\n", dir_path);
        CloseHandle(w->hDir);
        return false;
    }

    w->active = true;
    g_num_watchers++;
    printf("Watching directory: %s for extensions:", dir_path);
    for (int i = 0; i < num_extensions; ++i) printf(" %s", extensions[i]);
    printf("\n");

    return true;
}

void watcher_update(void)
{
    int head = g_queue_head;
    int tail = g_queue_tail;

    while (head != tail) {
        const char* path = g_reload_queue[head].shader_path;

        if (g_reload_callback)
            g_reload_callback(path);

        head = (head + 1) % MAX_RELOAD_QUEUE;
        InterlockedExchange(&g_queue_head, head);
    }
}

void watcher_cleanup(void)
{
    InterlockedExchange(&g_running, 0);
    thrd_join(g_watcher_thread, NULL);

    for (int i = 0; i < g_num_watchers; ++i) {
        DirectoryWatcher* w = &g_watchers[i];
        if (w->hDir != INVALID_HANDLE_VALUE) {
            CancelIo(w->hDir);
            CloseHandle(w->hDir);
        }
        free(w->config.dir_path);
        for (int j = 0; j < w->config.num_extensions; ++j)
            free(w->config.extensions[j]);
        free(w->config.extensions);
    }
    g_num_watchers = 0;
}