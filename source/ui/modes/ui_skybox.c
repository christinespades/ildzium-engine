#include "ui_skybox.h"
#include "ui/ui_elements.h"
#include "scene/sky.h"
#include "core/io.h"

void setup_skybox_controls(UI_Context* ctx)
{
    float btn_w = 380;
    float btn_h = 70;
    float start_x = 50;
    float start_y = 50;
    float spacing = btn_h;
    int idx = 0;

    ui_add_tuner(g_ui_ctx, start_x + 400, start_y + 4 * spacing, btn_w, btn_h,
                 "Aurora R", &g_skyParams.auroraColor[0], 0.0f, 1.0f);

    ui_add_tuner(g_ui_ctx, start_x + 400, start_y + 5 * spacing, btn_w, btn_h,
                 "Aurora G", &g_skyParams.auroraColor[1], 0.0f, 1.0f);

    ui_add_tuner(g_ui_ctx, start_x + 400, start_y + 6 * spacing, btn_w, btn_h,
                 "Aurora B", &g_skyParams.auroraColor[2], 0.0f, 1.0f);

    ui_add_tuner(g_ui_ctx, start_x + 400, start_y + 0 * spacing, btn_w, btn_h,
                 "Nebula Night R", &g_skyParams.nebulaColor1[0], 0.0f, 1.0f);

    ui_add_tuner(g_ui_ctx, start_x + 400, start_y + 1 * spacing, btn_w, btn_h,
                 "Nebula Night G", &g_skyParams.nebulaColor1[1], 0.0f, 1.0f);

    ui_add_tuner(g_ui_ctx, start_x + 400, start_y + 2 * spacing, btn_w, btn_h,
                 "Nebula Night B", &g_skyParams.nebulaColor1[2], 0.0f, 1.0f);

    // === Left column - Parameters ===
    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Aurora Intensity", &g_skyParams.auroraIntensity, 0.0f, 2.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Aurora Speed", &g_skyParams.auroraSpeed, 0.0f, 5.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Brightness", &g_skyParams.overallBrightness, 0.0f, 5.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Cycle Speed", &g_skyParams.cycleSpeed, 0.0f, 2.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Nebula Intensity", &g_skyParams.nebulaIntensity, 0.0f, 3.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Nebula Speed", &g_skyParams.nebulaSpeed, 0.0f, 10.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Nebula Scale", &g_skyParams.nebulaScale, 0.5f, 10.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Nebula Layers", &g_skyParams.nebulaLayerCount, 1.0f, 8.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Star Count", &g_skyParams.starCount, 0.0f, 2500.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Star Brightness", &g_skyParams.starBrightness, 0.0f, 5.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Star Twinkle", &g_skyParams.starTwinkleSpeed, 0.0f, 20.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Star Size", &g_skyParams.starSize, 0.001f, 0.3f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Time of Day", &g_skyParams.timeOfDay, 0.0f, 1.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Vignette", &g_skyParams.vignetteStrength, 0.0f, 1.5f);

    // === Scrollable Shader Editor on the right ===
    char* shader_src = load_file("../../shaders/sky.frag");
    ui_add_scrollable_text_editor(ctx,
        900, 50,               // x, y  (adjust to your resolution)
        800, 1200,              // width, height
        shader_src,
        "../../shaders/sky.frag");

    free(shader_src);          // ui_add_... made its own copy
}