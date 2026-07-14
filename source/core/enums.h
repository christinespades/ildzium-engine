#pragma once

// 1. The corrected generator macro (Notice: No nested #defines!)
#define DEFINE_ENUM_TO_STRING_FUNC(func_name, x_macro_map) \
    inline const char* func_name(int value) { \
        switch(value) { \
            x_macro_map \
            default: return NULL; \
        } \
    }

// 2. Your inline fallback helper remains exactly the same
static inline const char* enum_str(const char* (*lookup_func)(int), int value, const char* enum_name) {
    if (lookup_func) {
        const char* res = lookup_func(value);
        if (res) return res;
    }
    static char fallback_buf[128]; // Bumped slightly to prevent truncation bugs
    snprintf(fallback_buf, sizeof(fallback_buf), "(no strings defined for enum \"%s\" value %d)", enum_name, value);
    return fallback_buf;
}

typedef struct
{
    int value;
    const char* name;
} EnumStringEntry;


typedef struct
{
    const EnumStringEntry* entries;
    uint32_t count;

} EnumDefinition;

#include "input/keys/platform_key.h"
static const EnumStringEntry platform_key_names[] =
{
    { KEY_SPACE, "SPACE" },
    { KEY_A, "A" },
    { KEY_B, "B" },
    { KEY_C, "C" },
    { KEY_D, "D" },
    { KEY_W, "W" },
    { KEY_S, "S" },
    { KEY_ESCAPE, "ESCAPE" },
    { KEY_LEFT_SHIFT, "LEFT SHIFT" },
};

extern EnumDefinition enum_platform_key;

#include "ui/ui_button.h"


inline const char* enum_value_to_string(UI_Button* b)
{
    int value = *(int*)b->target_value;

    for (uint32_t i = 0; i < b->enum_definition->count; i++)
    {
        if (b->enum_definition->entries[i].value == value)
            return b->enum_definition->entries[i].name;
    }

    return "UNKNOWN";
}