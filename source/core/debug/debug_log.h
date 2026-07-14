#pragma once

static const char* g_debug_log_filter[] =
{
    "render"
};

static inline bool log_allowed(const char* file)
{
    for (uint32_t i = 0; i < sizeof(g_debug_log_filter) / sizeof(g_debug_log_filter[0]); i++)
    {
        if (strstr(file, g_debug_log_filter[i]))
            return false;
    }

    return true;
}

#define LOGI(fmt, ...)                                                   \
    do {                                                                 \
        const char* file = RELEST_PATH(__FILE__);                        \
        if (log_allowed(file))                                           \
            printf(ANSI_GREEN "\n%s:%d " fmt ANSI_RESET,                 \
                   file, __LINE__, ##__VA_ARGS__);                       \
    } while (0)

#define LOGE(fmt, ...)                                                   \
    do {                                                                 \
        const char* file = RELEST_PATH(__FILE__);                        \
        if (log_allowed(file))                                           \
            fprintf(stderr, ANSI_RED "\n%s:%d " fmt ANSI_RESET,          \
                    file, __LINE__, ##__VA_ARGS__);                      \
    } while (0)
    
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
#ifdef LOG_SCOPE_ENABLED
    #define LOG_SCOPE() \
        ScopedLogger __sl__ = { trim_source_path(__FILE__), __LINE__, __FUNCTION__, __ildz_logger_depth++ }; \
        if (__ildz_logger_depth < 5) __ildz_logger_depth = 0;
        __print_indent(__sl__.depth); \
        printf("[ENTER] %s:%d: %s\n", __sl__.file, __sl__.line, __sl__.func); \
        for (int __once__ = 0; !__once__; __once__ = 1, \
             __print_indent(__sl__.depth), \
             printf("[EXIT]  %s:%d: %s\n", __sl__.file, __sl__.line, __sl__.func), \
             __ildz_logger_depth--)
#else
    #define LOG_SCOPE() ((void)0);
#endif


#ifdef LOG_MALLOC_ENABLED
    #define LOG_MALLOC(sz) \
        (printf("[MALLOC] %s:%d: %zu bytes\n", trim_source_path(__FILE__), __LINE__, (size_t)(sz)), malloc(sz))
#else
    #define LOG_MALLOC(sz) malloc(sz)
#endif