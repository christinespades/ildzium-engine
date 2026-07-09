#include "pch.h"
#include "ui_params.h"

UI_ParamValue s_macro_param_registry[PARAM_COUNT] = { 0 };

void init_macro_param_registry(void)
{
    if (s_macro_registry_initialized) return;

    #define X(type, id, fallback, name_str, min, max) s_macro_param_registry[PARAM_##id].type = fallback;
    UI_PARAMS_MAP
    #undef X

    s_macro_registry_initialized = true;
}