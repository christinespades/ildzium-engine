#include "pch.h"
#include "ui_draw_info.h"

static float info_scroll_x = 0.0f;   // persistent horizontal scroll offset
extern float g_fps;

// Monospaced font width calculation
int get_text_width(const char* text, int scale)
{
    if (!text) return 0;
    int len = 0;
    while (text[len]) len++;
    return len * get_param_float(PARAM_UI_FONT_WIDTH) * scale;
}

// ====================== HORIZONTAL SCROLLING TEXT ======================
void draw_text_hscroll(uint32_t* fb, int fb_width, int fb_height,
                       const char* text,
                       int x, int y,
                       uint32_t color, int scale,
                       int scroll_offset_x,
                       int clip_width)
{
    if (!text || clip_width <= 0) return;

    const char* sep = " ||| ";

    int char_w = get_param_float(PARAM_UI_FONT_WIDTH) * scale;
    int text_len = strlen(text);
    int sep_len  = strlen(sep);

    int text_width = text_len * char_w;
    int sep_width  = sep_len * char_w;
    int total_width = text_width + sep_width;

    if (total_width == 0) return;

    int offset = scroll_offset_x % total_width;
    if (offset < 0) offset += total_width;

    int start_x = x - offset - total_width;

    for (int base_x = start_x; base_x < x + clip_width; base_x += total_width)
    {
        // draw main text
        for (int i = 0; i < text_len; i++)
        {
            int char_x = base_x + i * char_w;

            if (char_x + char_w > x && char_x < x + clip_width)
            {
                draw_char(fb, fb_width, fb_height, text[i], char_x, y, color, scale);
            }
        }

        // draw separator after text
        int sep_start_x = base_x + text_width;
        for (int i = 0; i < sep_len; i++)
        {
            int char_x = sep_start_x + i * char_w;

            if (char_x + char_w > x && char_x < x + clip_width)
            {
                draw_char(fb, fb_width, fb_height, sep[i], char_x, y, color, scale);
            }
        }
    }
}

void ui_draw_info(uint32_t* fb, int fb_width, int fb_height, float dt)
{
    // === Build info string dynamically ===
    char info_text[512] = {0};
    char* ptr = info_text;
    int remaining = sizeof(info_text) - 1;

    bool first = true;

    // Helper macro to make it clean and safe
    #define APPEND(fmt, ...) \
        do { \
            if (!first) { \
                int written = snprintf(ptr, remaining, " | "); \
                ptr += written; \
                remaining -= written; \
            } \
            int written = snprintf(ptr, remaining, fmt, __VA_ARGS__); \
            ptr += written; \
            remaining -= written; \
            first = false; \
        } while(0)
    if (g_project_flags & PROJECT_SHOW_NAME)
        APPEND("Project: %s", g_current_project_name);
    if (g_renderer_flags & RENDERER_SHOW_FPS)
        APPEND("FPS: %.1f", g_fps);

    if (g_renderer_flags & RENDERER_SHOW_DRAWN)
        APPEND("Drawn: %u", g_drawn_count);

    if (g_renderer_flags & RENDERER_SHOW_CULLED)
        APPEND("Culled: %u", g_culled_count);

    if (g_camera_flags & CAMERA_SHOW_POS)
        APPEND("Pos: %.1f, %.1f, %.1f", camera.x, camera.y, camera.z);

    if (g_camera_flags & CAMERA_SHOW_SPEED)
        APPEND("Speed: %.2f", camera.speed);
    if (g_camera_flags & CAMERA_SHOW_YAW_PITCH)
        APPEND("Yaw: %.1f  Pitch: %.1f", camera.yaw, camera.pitch);

    #undef APPEND

    // If nothing was added (shouldn't happen), early out
    if (info_text[0] == '\0')
        return;

    // === Panel setup ===
    const int panel_height = 32;
    const int panel_y = fb_height - panel_height;
    const int padding = 20;

    ui_draw_panel(fb, fb_width, fb_height,
                  0, panel_y, fb_width, panel_height,
                  get_param_color(PARAM_UI_COLOR_BOTTOM_INFO_PANEL_BG), get_param_color(PARAM_UI_COLOR_BOTTOM_INFO_PANEL_BORDER));

    // === Horizontal scrolling text ===
    int text_y = panel_y + (panel_height - get_param_float(PARAM_UI_FONT_HEIGHT)) / 2;
    int available_width = fb_width - 2 * padding;

    info_scroll_x += get_param_float(PARAM_UI_BOTTOM_INFO_PANEL_AUTOSCROLL_SPEED) * dt;

    // Draw
    draw_text_hscroll(fb, fb_width, fb_height,
                      info_text,
                      padding, text_y,
                      get_param_color(PARAM_UI_COLOR_BOTTOM_INFO_PANEL_TEXT), 1,
                      (int)info_scroll_x,
                      available_width);
}