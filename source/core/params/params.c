#include "pch.h"
#include "core/params/params.h"

ParamEntry s_macro_param_registry[PARAM_COUNT] = { 0 };

void init_macro_param_registry(void)
{
    if (s_macro_registry_initialized) return;

    #define X(type, id, name_str, min, max, tooltip) s_macro_param_registry[PARAM_##id].type = id;
    CAMERA_PARAMS_MAP
    CAMERA_PARAMS_V3_MAP
    ENGINE_PARAMS_MAP
    RENDER_PARAMS_MAP
    RENDER_PARAMS_V4_MAP
    INPUT_PARAMS_MAP
    SHADERS_SKY_PARAMS_MAP
    SHADERS_SKY_PARAMS_V3_MAP
    UI_PARAMS_MAP
    #undef X

    // enum binds
    #define X(type, id, name_str, value, enum_def, tooltip) \
        s_macro_param_registry[PARAM_##id].e = value; \
        s_macro_param_registry[PARAM_##id].enum_definition = enum_def;
    INPUT_BIND_PARAMS_MAP
    #undef X

    s_macro_registry_initialized = true;
}