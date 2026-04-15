#include "pch.h"
#include "ui_main.h"

static void on_readme_loaded(char* readme, void* user)
{
    if (!readme) return;

    LoadState* state = (LoadState*)user;
    UI_Context* ctx = state->ctx;

    int w, h;
    ildz_get_window_size(&w, &h);

    char processed[131072];
    char wrapped[131072];

    markdown_to_text(readme, processed, sizeof(processed));

    int max_chars = (w - 30) / 8;
    wrap_text(processed, wrapped, sizeof(wrapped), max_chars);

    // TODO: replace with render_markdown from ui_markdown.c after refactoring there (UI system needs specific element types for clickable links inside a scrollable text container, etc. so a scrollable text container contains a bunch of glyph elements and clickable link elements)
    ui_add_scrollable_text(ctx,
                    UI_CONTAINER_PADDING,
                    UI_CONTAINER_PADDING,
                    w - UI_CONTAINER_PADDING * 2,
                    h - UI_CONTAINER_PADDING * 2,
                    wrapped);

    free(state);
    free(readme);
}

void setup_main_controls(UI_Context* ctx)
{
    LoadState* state = malloc(sizeof(LoadState));
    state->ctx = ctx;
    platform_http_get(
        "https://raw.githubusercontent.com/christinespades/ildzium-engine/main/README.md",
        on_readme_loaded,
        state
    );
}