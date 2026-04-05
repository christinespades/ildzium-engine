#include "ui/ui_elements.h"
#include <stdlib.h>   // malloc, free
#include <string.h>   // strlen, strcpy

void ui_add_button(UI_Context* ctx, int x, int y, int w, int h, const char* text,
                   UI_ButtonCallback on_click,
                   UI_ButtonCallback on_held,
                   UI_ButtonCallback on_release) {
    if (ctx->button_count >= MAX_BUTTONS) return;

    UI_Button* b = &ctx->buttons[ctx->button_count];
    b->x = x; b->y = y; b->w = w; b->h = h;
    b->content = text;
    b->on_click = on_click;
    b->on_held = on_held;
    b->on_release = on_release;
    b->is_scrollable = false; // regular buttons don't have scroll

    // Default: no tuning
    b->target_value = NULL;
    b->step_size = 0.0f;
    b->min_value = 0.0f;
    b->max_value = 0.0f;

    ctx->button_count++;
}

void ui_add_scrollable_text(UI_Context* ctx, int x, int y, int w, int h, const char* text)
{
	if (ctx->button_count >= MAX_BUTTONS) return;
	UI_Button* b = &ctx->buttons[ctx->button_count];
	b->x = x; b->y = y; b->w = w; b->h = h;
	b->content = text;
	b->on_click = NULL;
	b->on_held = NULL;
	b->on_release = NULL;
	b->target_value = NULL;
	b->scroll_offset = 0.0f;
	b->content_height = 0.0f; // will be calculated during drawing
	b->is_scrollable = true;
	ctx->button_count++;
}

void ui_add_scrollable_text_editor(UI_Context* ctx, int x, int y, int w, int h, const char* initial_text, const char* filepath)
{
    if (ctx->button_count >= MAX_BUTTONS) return;
    UI_Button* b = &ctx->buttons[ctx->button_count];

    b->x = x; b->y = y; b->w = w; b->h = h;
    b->content = NULL;
    b->is_scrollable = true;
    b->is_editable = true;
    b->cursor_pos = 0;
    b->selection_start = b->selection_end = -1;
    b->scroll_offset = 0.0f;
    b->content_height = 0.0f;
    b->line_height = 20;        // tune to your font

    size_t len = initial_text ? strlen(initial_text) : 0;
    b->content_capacity = len + 4096;
	b->content = (char*)malloc(b->content_capacity);
	if (initial_text)
	    strcpy(b->content, initial_text);
	else
	    b->content[0] = '\0';
	
    b->filepath = _strdup(filepath);

    ctx->button_count++;
}

void ui_add_tuner(UI_Context* ctx, float x, float y, float w, float h,
                  const char* text,
                  float* target,
                  float min_val,
                  float max_val)
{
    if (ctx->button_count >= MAX_BUTTONS) return;

    UI_Button* b = &ctx->buttons[ctx->button_count];
    
    b->x = x; b->y = y; b->w = w; b->h = h;
    b->content = text;
    b->on_click = NULL;
    b->on_held = NULL;
    b->on_release = NULL;
    b->is_scrollable = false; // tuner buttons don't have scroll 
    b->target_value = target;
    b->min_value = min_val;
    b->max_value = max_val;
    // auto-compute (recommended)
    b->step_size = 0.0f;   // mark as "auto"
    
    ctx->button_count++;
}
