#pragma once
#include "core/tooltips.h"
// each ui ctx module (ui_main, ui_sounds) includes this. all modules are in turn included in ui_modes.h
// other ui modules also use it

// these values determine the sizes and amounts of chunks and should be as high as possible to allow smooth streaming. high numbers mean that we stream in small chunks instead of large ones. the binary save file must be deleted and the engine restarted for the change to take effect
#define MAX_CHUNKS_X 128
#define MAX_CHUNKS_Z 128

// cleanup_model_system and init_model_system must be called again for this to take effect
// vertex cap under 8 to 16 million vertices keeps you safe from memory index overflows
#define MAX_SCENE_VERTICES 2097152  // 2 million vertices (~64 MB VRAM)
#define MAX_SCENE_INDICES  4194304  // 4 million indices (~16 MB VRAM)

#include "core/params/params_camera.h"
#include "core/params/params_engine.h"
#include "core/params/params_input.h"
#include "core/params/params_render.h"
#include "core/params/params_shaders.h"
#include "core/params/params_ui.h"

typedef enum {
    #define X(type, id, name_str, ...) PARAM_##id,
    CAMERA_PARAMS_MAP
    CAMERA_PARAMS_V3_MAP
    ENGINE_PARAMS_MAP
    RENDER_PARAMS_MAP
    RENDER_PARAMS_V4_MAP
    INPUT_PARAMS_MAP
    INPUT_BIND_PARAMS_MAP
    SHADERS_SKY_PARAMS_MAP
    SHADERS_SKY_PARAMS_V3_MAP
    UI_PARAMS_MAP
    #undef X
    PARAM_COUNT
} MacroParamID;

typedef enum
{
    PARAM_ENUM_NONE,
    PARAM_ENUM_PLATFORM_KEY,

} ParamEnumType;

#include "core/enums.h"

typedef struct
{
    float f;
    uint32_t u;
    bool b;
    vec3 v3;
    vec4 v4;
    int e;
    EnumDefinition* enum_definition;

} ParamEntry;

// Update your global storage pool to use this new variant array
extern ParamEntry s_macro_param_registry[PARAM_COUNT];
static bool s_macro_registry_initialized = false;

void init_macro_param_registry(void);

static inline vec3 get_param_vec3(MacroParamID param_id)
{
    return s_macro_param_registry[param_id].v3;
}

static inline vec3* get_param_vec3_ptr(MacroParamID id)
{
    return &s_macro_param_registry[id].v3;
}

static inline vec4 get_param_vec4(MacroParamID param_id)
{
    return s_macro_param_registry[param_id].v4;
}

static inline vec4* get_param_vec4_ptr(MacroParamID id)
{
    return &s_macro_param_registry[id].v4;
}

static inline bool get_param_bool(MacroParamID param_id) {
    return s_macro_param_registry[param_id].b;
}

static inline float get_param_float(MacroParamID param_id) {
    return s_macro_param_registry[param_id].f;
}

static inline float* get_param_float_ptr(MacroParamID id)
{
    return &s_macro_param_registry[id].f;
}

static inline uint32_t get_param_color(MacroParamID param_id) {
    // Return the clean, unmutated 32-bit integer color sequence
    return s_macro_param_registry[param_id].u;
}

static inline uint32_t get_param_uint(MacroParamID param_id) {
    return s_macro_param_registry[param_id].u;
}

static inline int get_param_enum(MacroParamID param_id)
{
    return s_macro_param_registry[param_id].e;
}
