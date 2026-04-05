#include "io.h"
#include <stdio.h>    // FILE, fopen, fclose, fread, fseek, ftell, SEEK_END, SEEK_SET
#include <stdlib.h>   // malloc, free
#include <string.h>   // strlen, strcpy

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

char* save_file(const char* path, const char* data)
{
    if (!data) return NULL;  // nothing to write

    FILE* f = fopen(path, "wb");
    if (!f) {
        const char* err_msg = "// Failed to save file\n";
        char* buf = (char*)malloc(strlen(err_msg) + 1);
        strcpy(buf, err_msg);
        return buf;
    }

    size_t len = strlen(data);
    fwrite(data, 1, len, f);
    fclose(f);

    // Return a simple success message, optional
    const char* success_msg = "// File saved successfully\n";
    char* buf = (char*)malloc(strlen(success_msg) + 1);
    strcpy(buf, success_msg);
    return buf;
}