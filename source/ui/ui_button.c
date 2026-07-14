#include "pch.h"
#include "ui_button.h"
#include "ui_context.h" // avoid circular dependency so we include it here

// for default buttons
void ui_add_button(UI_Context* ctx, int x, int y, int w, int h, const char* text,
                   UI_ButtonCallback on_click,
                   UI_ButtonCallback on_held,
                   UI_ButtonCallback on_release,
                   UI_ButtonCallback on_hover,
                   const char* tooltip_text) {
    if (ctx->button_count >= get_param_float(PARAM_UI_MAX_BUTTONS)) return;

    UI_Button* b = &ctx->buttons[ctx->button_count];
    b->x = x; b->y = y; b->w = w; b->h = h;
    // If text is provided, bake it into the button's local stable storage
    if (text) {
        strncpy(b->text_storage, text, sizeof(b->text_storage) - 1);
        b->text_storage[sizeof(b->text_storage) - 1] = '\0';
        b->content = b->text_storage; // Point content to its own safe local memory
    } else {
        b->text_storage[0] = '\0';
        b->content = NULL; // Safe for layout containers assigning NULL
    }
    b->editable_content = NULL;
    b->is_editable = false;
    b->on_click = on_click;
    b->on_held = on_held;
    b->on_release = on_release;
    b->on_hover = on_hover;
    b->tooltip = tooltip_text;
    b->is_scrollable = false; // regular buttons don't have scroll
    b->target_value = NULL; // Default: no tuning. set target_value for tuners only
    b->type = PARAM_TYPE_NONE;
    b->step_size = 0.0f;
    b->min_value = 0.0f;
    b->max_value = 0.0f;
    b->is_container = false; // only for layout boxes
    b->is_typing = false; // only for tuner text editor boxes
    ctx->button_count++;
}

void add_top_button(UI_Context *ctx,
                           int *x,
                           int top_y,
                           int btn_w,
                           int spacing,
                           const char *label,
                           UI_ButtonCallback on_click,
                           const char* tooltip)
{
    ui_add_button(ctx, *x, top_y, btn_w, 50,
                  label,
                  on_click,
                  on_button_held,
                  NULL,
                  NULL,
                  tooltip);

    *x += btn_w + spacing;
}

#define BUTTON_LIST \
    X("CAMERA",  camera)  \
    X("DEBUG", debug) \
    X("FX",      fx)      \
    X("INPUT",   input)   \
    X("LIGHTS",  lights)  \
    X("MAIN",    main)    \
    X("MESHES",  meshes)  \
    X("PROJECT", project) \
    X("RENDER", render) \
    X("SHADERS",  shaders)  \
    X("SOUNDS",  sounds)  \
    X("TERRAIN", terrain) \
    X("UI",      ui)

void add_top_navigation_buttons(UI_Context* ctx) {
    const int top_y = 5;
    const int btn_w = 200;
    const int spacing = 8;
    int x = get_param_float(PARAM_UI_ELEMENT_START_X);

    #define X(label, cb) \
        add_top_button(ctx, &x, top_y, btn_w, spacing, label, on_##cb##_clicked, tooltip_menu_##cb);

    BUTTON_LIST

    #undef X
}

bool ui_button_has_focus(UI_Button* b)
{
    uint32_t index = (uint32_t)(b - g_ui_ctx->buttons);
    return index == g_ui_ctx->active_button_index;
}