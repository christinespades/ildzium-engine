#include "pch.h"
#include "ui_update.h"

// New helper: select entire current line
void select_entire_line(UI_Button* b, int click_pos)
{
    const char* text = b->editable_content;
    if (!text) return;
    int len = (int)strlen(text);

    int start = click_pos;
    while (start > 0 && text[start-1] != '\n') start--;

    int end = click_pos;
    while (end < len && text[end] != '\n') end++;

    b->selection_start = start;
    b->selection_end = end;
    b->cursor_pos = end;
}

void ui_button_mouse_drag(UI_Button* b)
{
    if (!b->is_dragging || !b->is_editable) return;

    int current_pos = get_char_index_from_mouse(b);

    if (b->click_count == 1)   // normal drag selection
    {
        b->selection_start = b->drag_start_pos;
        b->selection_end = current_pos;
        b->cursor_pos = current_pos;
    }
    else if (b->click_count == 2)   // word-by-word drag after double-click
    {
        // Optional: expand word selection while dragging (many editors do this)
        // For simplicity you can keep normal selection here
        b->selection_start = b->drag_start_pos;
        b->selection_end = current_pos;
        b->cursor_pos = current_pos;
    }
}

void ui_button_mouse_up(UI_Button* b)
{
    b->is_dragging = false;
}
static int ui_button_is_hovered(const UI_Button* b)
{
    return (g_mouse_x >= b->x && g_mouse_x < b->x + b->w &&
            g_mouse_y >= b->y && g_mouse_y < b->y + b->h);
}

static void ui_handle_release(UI_Context* ctx)
{
    UI_Button* active = &ctx->buttons[ctx->active_button_index];
    if (ctx->active_button_index >= 0 && ctx->active_button_index < ctx->button_count){
        ui_button_mouse_up(active);
        if (active->on_release) {
            ((void (*)(UI_Button*))active->on_release)(active);
        }
    }
    if (active->is_editable && ui_button_is_hovered(active)) {
    	// prevent editable text from losing focus
    	return;
    }
	//LOGI("Editable tuner button lost focus");
    active->is_typing=false;
    active->click_count = 0;
    //LOGI("Button lost focus");
    ctx->active_button_index = -1;
}

static bool ui_try_capture_button(UI_Context* ctx, UI_Button* b, int index, int is_hover, int mouse_pressed_this_frame)
{
    if (mouse_pressed_this_frame && is_hover && ctx->active_button_index == -1) {
        ctx->active_button_index = index;
        LOGI("Marked %s with index %u as the active button", b->content, ctx->active_button_index);

        // Mark the press state for per-button logic.
        b->is_dragging = b->is_editable;
        b->hold_time = 0.0f;
        b->last_click_time = ildz_get_time();

        if (b->on_click) {
            ((void (*)(UI_Button*))b->on_click)(b);
            return true;
        }
        return false;
    }
    return false;
}

void ui_update(UI_Context* ctx, int mouse_pressed, int mousewheel)
{
    if (!ctx->cursor_captured) return;

    int mouse_pressed_this_frame = mouse_pressed && !ctx->mouse_pressed_last_frame;
    int mouse_released_this_frame = !mouse_pressed && ctx->mouse_pressed_last_frame;

    if (mouse_released_this_frame) {
        ui_handle_release(ctx);
    }

    int input_consumed = 0;

    for (int i = ctx->button_count - 1; i >= 0; i--) {
        UI_Button* b = &ctx->buttons[i];
        int is_hover = !input_consumed && ui_button_is_hovered(b);
        int click_here = is_hover && mouse_pressed_this_frame;
        if (click_here) {
            input_consumed = 1;
            ctx->active_button_index = -1;
        }

        if (b->is_scrollable && is_hover && mousewheel != 0) {
            b->scroll_offset -= mousewheel * 25.0f;   
        }

        if (ui_try_capture_button(ctx, b, i, is_hover, click_here)) continue;

        if (b->is_editable && ctx->active_button_index == i) {
            ui_update_editable_button(b, mouse_pressed, mouse_pressed_this_frame);
        }

        if (ctx->active_button_index == i) {
            ui_update_tuners(b, mouse_pressed, mouse_pressed_this_frame);
        }

        ui_dispatch_callbacks(
            b,
            ctx->active_button_index == i,
            mouse_pressed,
            click_here,
            mouse_released_this_frame
        );
    }

    ctx->mouse_pressed_last_frame = mouse_pressed;
}