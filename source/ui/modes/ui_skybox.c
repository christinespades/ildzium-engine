#include "pch.h"
#include "ui_skybox.h"
#include "ui/ui_elements.h"
#include "ui/ui_layout.h"
#include "scene/sky.h"
#include "core/io.h"

#define SKY_PARAMS_MAP \
    X(&g_skyParams.auroraColor[0],      "Aurora R",                  -255.0f,  255.0f)  \
    X(&g_skyParams.auroraColor[1],      "Aurora G",                  -255.0f,  255.0f)  \
    X(&g_skyParams.auroraColor[2],      "Aurora B",                  -255.0f,  255.0f)  \
    X(&g_skyParams.nebulaColor1[0],     "Nebula Night R",               0.0f,    1.0f)  \
    X(&g_skyParams.nebulaColor1[1],     "Nebula Night G",               0.0f,    1.0f)  \
    X(&g_skyParams.nebulaColor1[2],     "Nebula Night B",               0.0f,    1.0f)  \
    X(&g_skyParams.auroraIntensity,     "Aurora Intensity",           -20.0f,   20.0f)  \
    X(&g_skyParams.auroraSpeed,         "Aurora Speed",                 0.0f,    5.0f)  \
    X(&g_skyParams.overallBrightness,   "Brightness",                   0.0f,    5.0f)  \
    X(&g_skyParams.cycleSpeed,          "Cycle Speed",                  0.0f,    2.0f)  \
    X(&g_skyParams.nebulaIntensity,     "Nebula Intensity",           -20.0f,   20.0f)  \
    X(&g_skyParams.nebulaSpeed,         "Nebula Speed",               -20.0f,   20.0f)  \
    X(&g_skyParams.nebulaScale,         "Nebula Scale",               -20.0f,   20.0f)  \
    X(&g_skyParams.nebulaLayerCount,    "Nebula Layers",              -20.0f,   20.0f)  \
    X(&g_skyParams.starCount,           "No Effect (Stars)",            0.0f, 2500.0f)  \
    X(&g_skyParams.starBrightness,      "No Effect (Brightness)",       0.0f,    5.0f)  \
    X(&g_skyParams.starTwinkleSpeed,    "Vertical Cloud Brightness",  -20.0f,   20.0f)  \
    X(&g_skyParams.starSize,            "Vertical Cloud Speed",       -10.0f,   10.0f)  \
    X(&g_skyParams.timeOfDay,           "Time of Day",                  0.0f,    1.0f)  \
    X(&g_skyParams.vignetteStrength,    "Vignette",                     0.0f,    1.5f)

void setup_skybox_controls(UI_Context* ctx)
{
    UI_Layout l = ui_layout_begin(ctx);

    #define X(target_ptr, name_str, min_val, max_val) \
        ui_add_tuner(ctx, l.current_x, ui_layout_next_y(&l), l.btn_w, l.btn_h, \
                     name_str, target_ptr, min_val, max_val);
    
    SKY_PARAMS_MAP
    #undef X

    // === Scrollable Shader Editor on the right ===
    char* shader_src = load_file("../../shaders/sky.frag");
    ui_add_scrollable_text_editor(ctx,
        900, 70,
        get_param_float(PARAM_UI_EDITOR_WIDTH), get_param_float(PARAM_UI_EDITOR_HEIGHT),
        shader_src,
        "../../shaders/sky.frag");

    free(shader_src);          // ui_add_... made its own copy
}