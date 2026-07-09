#include "pch.h"
#include "platform.h"

struct PlatformDir {
#ifdef _WIN32
    HANDLE hFind;
    WIN32_FIND_DATAA findData;
#else
    DIR* handle;
    struct dirent* entry;
    char path[512]; // Store path to check file stats
#endif
    bool first;
};

void platform_close_dir(PlatformDir* dir)
{
    if (dir) {
#ifdef _WIN32
        if (dir->hFind != INVALID_HANDLE_VALUE)
            FindClose(dir->hFind);
#else
        if (dir->handle)
            closedir(dir->handle);
#endif
        free(dir);
    }
}

bool platform_directory_exists(const char* path)
{
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return (stat(path, &st) == 0) && S_ISDIR(st.st_mode);
#endif
}

bool platform_rename(const char* old_path, const char* new_path)
{
#ifdef _WIN32
    return MoveFileA(old_path, new_path) != 0;
#else
    return rename(old_path, new_path) == 0;
#endif
}

bool platform_delete_directory(const char* path)
{
#ifdef _WIN32
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "rmdir /s /q \"%s\"", path);
    return system(cmd) == 0;
#else
    // POSIX rmdir only works on empty dirs. 
    // For recursive delete, you'd usually need a custom walker.
    return rmdir(path) == 0;
#endif
}

PlatformDir* platform_open_dir(const char* path) {
    PlatformDir* dir = (PlatformDir*)malloc(sizeof(PlatformDir));
    if (!dir) return NULL;
    dir->first = true;

#ifdef _WIN32
    char search_path[512];
    snprintf(search_path, sizeof(search_path), "%s\\*", path);
    dir->hFind = FindFirstFileA(search_path, &dir->findData);
    if (dir->hFind == INVALID_HANDLE_VALUE) {
        free(dir);
        return NULL;
    }
#else
    dir->handle = opendir(path);
    strncpy(dir->path, path, sizeof(dir->path) - 1);
    if (!dir->handle) {
        free(dir);
        return NULL;
    }
#endif
    return dir;
}

bool platform_read_dir(PlatformDir* dir, PlatformDirEntry* entry) {
    if (!dir || !entry) return false;

    while (true) {
#ifdef _WIN32
        if (dir->first) dir->first = false;
        else if (!FindNextFileA(dir->hFind, &dir->findData)) return false;

        const char* name = dir->findData.cFileName;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

        strncpy(entry->name, name, sizeof(entry->name) - 1);
        entry->is_directory = (dir->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
        dir->entry = readdir(dir->handle);
        if (!dir->entry) return false;

        const char* name = dir->entry->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

        strncpy(entry->name, name, sizeof(entry->name) - 1);

        // Get file info to see if it's a directory
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir->path, name);
        struct stat st;
        stat(full_path, &st);
        entry->is_directory = S_ISDIR(st.st_mode);
#endif
        return true;
    }
}

bool platform_create_directory(const char* path) {
#ifdef _WIN32
    return CreateDirectoryA(path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    return mkdir(path, 0777) == 0 || errno == EEXIST;
#endif
}

uint64_t platform_get_file_time(const char* path) {
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA(path, GetFileExInfoStandard, &data)) {
        ULARGE_INTEGER ull;
        ull.LowPart = data.ftLastWriteTime.dwLowDateTime;
        ull.HighPart = data.ftLastWriteTime.dwHighDateTime;
        return ull.QuadPart;
    }
#else
    struct stat st;
    if (stat(path, &st) == 0) {
        return (uint64_t)st.st_mtime;
    }
#endif
    return 0;
}

char* ildz_strdup(const char* str)
{
    if (!str) return NULL;
    size_t len = strlen(str) + 1;
    char* copy = (char*)malloc(len);
    if (copy) memcpy(copy, str, len);
    return copy;
}