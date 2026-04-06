#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct { const char* file; int line; const char* func; int depth; } ScopedLogger;

// Global call depth counter
static int __ildz_logger_depth = 0;

// Trim file path to relative to "source\" folder
static inline const char* trim_source_path(const char* path) {
    const char* src = strstr(path, "source\\");
    return src ? src : path;
}

// Helper: print indentation
static inline void __print_indent(int depth) {
    for (int i = 0; i < depth; i++) printf("  ");  // two spaces per depth
}

// RAII-style macro: logs ENTER at start, EXIT at end with indentation
#define LOG_SCOPE() \
    ScopedLogger __sl__ = { trim_source_path(__FILE__), __LINE__, __FUNCTION__, __ildz_logger_depth++ }; \
    __print_indent(__sl__.depth); \
    printf("[ENTER] %s:%d: %s\n", __sl__.file, __sl__.line, __sl__.func); \
    for (int __once__ = 0; !__once__; __once__ = 1, \
         __print_indent(__sl__.depth), \
         printf("[EXIT]  %s:%d: %s\n", __sl__.file, __sl__.line, __sl__.func), \
         __ildz_logger_depth--)

// Optional: logging mallocs, trimmed file path
#define LOG_MALLOC(sz) \
    (printf("[MALLOC] %s:%d: %zu bytes\n", trim_source_path(__FILE__), __LINE__, (size_t)(sz)), malloc(sz))
