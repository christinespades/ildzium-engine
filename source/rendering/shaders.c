#include <stdio.h>    // FILE, fopen, fread, fseek, ftell, fclose, printf
#include <stdlib.h>   // malloc, free, exit, NULL
#include <stdint.h>   // uint32_t
#include <string.h>
#include "scene/model.h"
#include "scene/sky.h"
#include "ui/ui.h"

uint32_t* load_spirv(const char* filename, size_t* out_size)
{
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open shader: %s\n", filename);
        *out_size = 0;
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size % 4 != 0) {
        printf("Invalid SPIR-V file size: %s\n", filename);
        fclose(file);
        *out_size = 0;
        return NULL;
    }

    uint32_t* code = malloc(file_size);
    fread(code, 1, file_size, file);
    fclose(file);

    *out_size = file_size;
    return code;
}

typedef void (*pipeline_fn)(void);

extern void create_model_pipeline(void);
extern void create_ui_pipeline(void);
extern void create_sky_pipeline(void);

typedef struct {
    const char* prefix;
    size_t      prefix_len;
    pipeline_fn fn;
} ShaderDispatch;

static const ShaderDispatch dispatch_table[] = {
    { "model", sizeof("model") - 1, create_model_pipeline },
    { "ui", sizeof("ui") - 1, create_ui_pipeline    },
    { "sky", sizeof("sky") - 1, create_sky_pipeline   },
};

static const size_t dispatch_count =
    sizeof(dispatch_table) / sizeof(dispatch_table[0]);

static const char* get_filename(const char* path)
{
    const char* slash1 = strrchr(path, '/');
    const char* slash2 = strrchr(path, '\\');
    const char* slash = slash1 > slash2 ? slash1 : slash2;
    return slash ? slash + 1 : path;
}

void on_shader_changed(const char* path)
{
    const char* filename = get_filename(path);

    printf("[Renderer] Hot-reloading shader: %s\n", filename);

    for (size_t i = 0; i < dispatch_count; ++i)
    {
        const ShaderDispatch* entry = &dispatch_table[i];

        if (strncmp(filename, entry->prefix, entry->prefix_len) == 0)
        {
            entry->fn();
            return;
        }
    }

    printf("[Renderer] No pipeline mapped for: %s\n", filename);
}