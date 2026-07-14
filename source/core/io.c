#include "pch.h"
#include "io.h"

char* load_file(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (!f) {
        // fallback
        const char* fallback = "// Failed to load file\n";
        char* buf = (char*)malloc(strlen(fallback) + 1);
        strcpy(buf, fallback);
        return buf;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buf = (char*)malloc(len + 1);
    fread(buf, 1, len, f);
    fclose(f);
    buf[len] = '\0';
    return buf;
}

void save_file(const char *path, const char *data)
{
    if (!path || path[0] == '\0') {
        LOGE("Error: Invalid file path.");
        return;
    }

    if (!data) {
        LOGE("Error: File data is NULL.");
        return;
    }

    // --- 1. Extract the Directory and Create It ---
    // Make a mutable copy of the path to manipulate it safely
    char *path_copy = strdup(path);
    if (path_copy) {
        // Find the last trailing slash (handles both Windows '\' and Unix '/')
        char *last_slash = strrchr(path_copy, '/');
        #ifdef _WIN32
        char *last_backslash = strrchr(path_copy, '\\');
        if (last_backslash > last_slash) {
            last_slash = last_backslash;
        }
        #endif

        if (last_slash != NULL) {
            *last_slash = '\0'; // Temporarily truncate the string at the last slash to isolate the folder path
            
            // If the directory path isn't empty, create it
            if (strlen(path_copy) > 0) {
                if (!platform_create_directory(path_copy)) {
                    LOGE("Warning: Could not verify/create directory '%s'", path_copy);
                    // We don't return here because the directory might already exist 
                    // via a complex edge-case, let fopen be the ultimate judge.
                }
            }
        }
        free(path_copy);
    }

    // --- 2. Proceed with File Writing ---
    FILE *f = fopen(path, "wb");
    if (!f) {
        LOGE("Failed to open '%s': ", path);
        perror(NULL);
        return;
    }

    size_t len = strlen(data);
    size_t written = fwrite(data, 1, len, f);

    if (written != len) {
        LOGE("Failed to write file '%s' (%zu of %zu bytes written).", path, written, len);
        fclose(f);
        return;
    }

    if (fclose(f) != 0) {
        LOGE("Failed to close '%s': ", path);
        perror(NULL);
        return;
    }

    LOGI("Saved %zu bytes to '%s'", len, path);
}