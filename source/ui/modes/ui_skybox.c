#include "pch.h"
#include "ui_skybox.h"
#include "ui/ui_elements.h"
#include "ui/ui_params.h"
#include "scene/sky.h"
#include "core/io.h"

void setup_skybox_controls(UI_Context* ctx)
{
    float btn_w = 280;
    float btn_h = 50;
    float start_x = 20;
    float start_y = 70;
    float spacing = btn_h;
    int idx = 0;

    ui_add_tuner(g_ui_ctx, start_x + 400, start_y + 4 * spacing, btn_w, btn_h,
                 "Aurora R", &g_skyParams.auroraColor[0], -255.0f, 255.0f);

    ui_add_tuner(g_ui_ctx, start_x + 400, start_y + 5 * spacing, btn_w, btn_h,
                 "Aurora G", &g_skyParams.auroraColor[1], -255.0f, 255.0f);

    ui_add_tuner(g_ui_ctx, start_x + 400, start_y + 6 * spacing, btn_w, btn_h,
                 "Aurora B", &g_skyParams.auroraColor[2], -255.0f, 255.0f);

    ui_add_tuner(g_ui_ctx, start_x + 400, start_y + 0 * spacing, btn_w, btn_h,
                 "Nebula Night R", &g_skyParams.nebulaColor1[0], 0.0f, 1.0f);

    ui_add_tuner(g_ui_ctx, start_x + 400, start_y + 1 * spacing, btn_w, btn_h,
                 "Nebula Night G", &g_skyParams.nebulaColor1[1], 0.0f, 1.0f);

    ui_add_tuner(g_ui_ctx, start_x + 400, start_y + 2 * spacing, btn_w, btn_h,
                 "Nebula Night B", &g_skyParams.nebulaColor1[2], 0.0f, 1.0f);

    // === Left column - Parameters ===
    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Aurora Intensity", &g_skyParams.auroraIntensity, -20.0f, 20.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Aurora Speed", &g_skyParams.auroraSpeed, 0.0f, 5.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Brightness", &g_skyParams.overallBrightness, 0.0f, 5.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Cycle Speed", &g_skyParams.cycleSpeed, 0.0f, 2.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Nebula Intensity", &g_skyParams.nebulaIntensity, -20.0f, 20.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Nebula Speed", &g_skyParams.nebulaSpeed, -20.0f, 20.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Nebula Scale", &g_skyParams.nebulaScale, -20.0f, 20.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Nebula Layers", &g_skyParams.nebulaLayerCount, 20.0f, 20.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "No Effect...", &g_skyParams.starCount, 0.0f, 2500.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "No Effect...", &g_skyParams.starBrightness, 0.0f, 5.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Vertical Cloud Brightness", &g_skyParams.starTwinkleSpeed, -20.0f, 20.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Vertical Cloud Speed", &g_skyParams.starSize, -10.0f, 10.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Time of Day", &g_skyParams.timeOfDay, 0.0f, 1.0f);

    ui_add_tuner(g_ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Vignette", &g_skyParams.vignetteStrength, 0.0f, 1.5f);

    // === Scrollable Shader Editor on the right ===
    char* shader_src = load_file("../../shaders/sky.frag");
    ui_add_scrollable_text_editor(ctx,
        900, 70,               // x, y  (adjust to your resolution)
        EDITOR_WIDTH, EDITOR_HEIGHT,
        shader_src,
        "../../shaders/sky.frag");

    free(shader_src);          // ui_add_... made its own copy
}