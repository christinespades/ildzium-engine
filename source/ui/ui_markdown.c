#include "pch.h"
#include "ui/ui_markdown.h"

/* TODO: Uncomment this render_markdown, replace ui_add_scrollable_text in ui_main.c with this, after adding:
- void ui_add_glyph(UI_Context* ctx, int x, int y, char c, float scale, bool bold, bool italic); void - ui_add_text_link(UI_Context* ctx, int x, int y, const char* text, int len, const char* url, int url_len);
- void platform_open_url(const char* url)

void render_markdown(UI_Context* ctx, int x, int y, int w, int h, const char* text)
{
    int cursor_x = x;
    int cursor_y = y;

    float scale = 1.0f;
    bool bold = false;
    bool italic = false;

    const char* p = text;

    while (*p)
    {
        unsigned char c = *p;

        // Headers (your existing system)
        if (c >= 0x02 && c <= 0x07)
        {
            int level = c - 0x01;
            scale = 1.6f - (level * 0.15f);
            p++;
            continue;
        }

        // Bold
        if (c == MD_BOLD_START) { bold = true; p++; continue; }
        if (c == MD_BOLD_END)   { bold = false; p++; continue; }

        // Italic
        if (c == MD_ITALIC_START) { italic = true; p++; continue; }
        if (c == MD_ITALIC_END)   { italic = false; p++; continue; }

        // Links
        if (c == MD_LINK_START)
        {
            p++;

            const char* link_text = p;
            int text_len = strlen(p);

            while (*p) p++;
            p++; // skip null separator

            const char* url = p;
            int url_len = strlen(p);

            while (*p) p++;

            // render clickable text
            ui_add_text_link(ctx,
                cursor_x, cursor_y,
                link_text, text_len,
                url, url_len);

            p++; // skip MD_LINK_END
            continue;
        }

        // Newline
        if (c == '\n')
        {
            cursor_y += (int)(20 * scale);
            cursor_x = x;
            p++;
            continue;
        }

        // Normal char
        ui_add_glyph(ctx, cursor_x, cursor_y, c, scale, bold, italic);
        cursor_x += (int)(8 * scale);

        p++;
    }
}
*/