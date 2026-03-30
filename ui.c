#include "ui.h"
#include <string.h>
#include <stdlib.h>

#define MAX_BUTTONS 64

static uint8_t font8x8_basic[128][8] = {
    // Minimal ASCII 32..127 bitmap font
    // For brevity, fill only A-Z, 0-9, basic symbols; rest zero
    // Each byte is one row of 8 pixels, 1 = pixel on, 0 = off
    // For example, 'A':
    [65] = {0x18,0x24,0x42,0x42,0x7E,0x42,0x42,0x00},
    // 'B':
    [66] = {0x7C,0x42,0x42,0x7C,0x42,0x42,0x7C,0x00},
    // ... fill as needed
};

void ui_init(UI_Context* ctx) {
    ctx->buttons = malloc(sizeof(UI_Button) * MAX_BUTTONS);
    ctx->button_count = 0;
    ctx->cursor_visible = 0;
}

void ui_add_button(UI_Context* ctx, int x, int y, int w, int h, const char* text, void (*on_click)(void)) {
    if (ctx->button_count >= MAX_BUTTONS) return;
    UI_Button* b = &ctx->buttons[ctx->button_count++];
    b->x = x; b->y = y; b->w = w; b->h = h;
    b->text = text;
    b->on_click = on_click;
}

static void draw_pixel(uint32_t* fb, int fb_w, int fb_h, int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= fb_w || y >= fb_h) return;
    fb[y * fb_w + x] = color;
}

static void draw_rect(uint32_t* fb, int fb_w, int fb_h, int x, int y, int w, int h, uint32_t color) {
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            draw_pixel(fb, fb_w, fb_h, x+i, y+j, color);
}

static void draw_char(uint32_t* fb, int fb_w, int fb_h, char c, int x, int y, uint32_t color) {
    if ((unsigned char)c >= 128) return;
    for (int row = 0; row < 8; row++) {
        uint8_t bits = font8x8_basic[(unsigned char)c][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << (7-col))) draw_pixel(fb, fb_w, fb_h, x+col, y+row, color);
        }
    }
}

static void draw_text(uint32_t* fb, int fb_w, int fb_h, const char* text, int x, int y, uint32_t color) {
    int cursor_x = x;
    while (*text) {
        draw_char(fb, fb_w, fb_h, *text++, cursor_x, y, color);
        cursor_x += 8; // 8 pixels per char
    }
}

void ui_update(UI_Context* ctx, int mouse_x, int mouse_y, int mouse_pressed) {
    if (!ctx->cursor_visible) return;
    for (int i = 0; i < ctx->button_count; i++) {
        UI_Button* b = &ctx->buttons[i];
        if (mouse_pressed &&
            mouse_x >= b->x && mouse_x < b->x+b->w &&
            mouse_y >= b->y && mouse_y < b->y+b->h) {
            if (b->on_click) b->on_click();
        }
    }
}

void ui_draw(UI_Context* ctx, uint32_t* fb, int fb_width, int fb_height) {
    if (!ctx->cursor_visible) return;

    // draw black background
    for (int i = 0; i < fb_width*fb_height; i++)
        fb[i] = 0xFF000000; // opaque black

    // draw buttons
    for (int i = 0; i < ctx->button_count; i++) {
        UI_Button* b = &ctx->buttons[i];
        draw_rect(fb, fb_width, fb_height, b->x, b->y, b->w, b->h, 0xFFFFFFFF); // white button
        draw_text(fb, fb_width, fb_height, b->text, b->x+2, b->y+2, 0xFF000000); // black text
    }
}