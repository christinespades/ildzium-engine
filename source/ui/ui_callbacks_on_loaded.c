#include "pch.h"
#include "ui_callbacks_on_loaded.h"

void on_readme_loaded(char* readme, void* user)
{
    LoadState* state = user;

    if (!readme) {
        free(state);
        return;
    }
    UI_Context* ctx = state->ctx;

    int w, h;
    ildz_get_window_size(&w, &h);

    char processed[131072];
    char wrapped[131072];

    markdown_to_text(readme, processed, sizeof(processed));

    int max_chars = (w - 30) / 8;
    wrap_text(processed, wrapped, sizeof(wrapped), max_chars);

    // TODO: replace with render_markdown from ui_markdown.c after refactoring there (UI system needs specific element types for clickable links inside a scrollable text container, etc. so a scrollable text container contains a bunch of glyph elements and clickable link elements)
    float pad = get_param_float(PARAM_UI_CONTAINER_PADDING);
    float x = get_param_float(PARAM_UI_ELEMENT_START_X);
    float y = get_param_float(PARAM_UI_ELEMENT_START_Y);
    ui_add_scrollable_text_editor(ctx,
        x, y,
        w - pad * 2,
        h - pad * 2,
        wrapped,
        "../../notes/readme.txt");
    free(state);
}