#include "pch.h"
#include "ui/ui_editor_clipboard.h"

#ifdef __EMSCRIPTEN__
    EM_JS(void, js_set_clipboard, (const char* text), {
        const str = UTF8ToString(text);
        navigator.clipboard.writeText(str).catch(() => {});
    });

    EM_JS(void, js_get_clipboard, (void (*cb)(const char*, void*), void* user), {
        navigator.clipboard.readText()
            .then(text => {
                const len = lengthBytesUTF8(text) + 1;
                const buf = _malloc(len);
                stringToUTF8(text, buf, len);
                dynCall_vii(cb, buf, user);
                _free(buf);
            })
            .catch(() => {
                dynCall_vii(cb, 0, user);
            });
    });
#endif

void platform_set_clipboard(const char* text)
{
    #ifdef __EMSCRIPTEN__
        js_set_clipboard(text);
    #else
        glfwSetClipboardString(g_window, text);
    #endif
}

void platform_get_clipboard_async(void (*callback)(const char*, void*), void* user)
{
    #ifdef __EMSCRIPTEN__
        js_get_clipboard(callback, user);
    #else
        const char* text = glfwGetClipboardString(g_window);
        callback(text ? text : "", user);
    #endif
}

static void paste_callback(const char* text, void* user)
{
    UI_Button* b = (UI_Button*)user;
    if (!b || !text || !*text) return;

    insert_string_at_cursor(b, text);
}

void paste_from_clipboard(UI_Button* b)
{
    if (!b->is_editable || !b->editable_content) return;

    platform_get_clipboard_async(paste_callback, b);
}

void copy_selection_to_clipboard(UI_Button* b) {
    if (b->selection_start == -1 || !b->editable_content) return;

    int start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
    int end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;

    if (start == end) return;

    char* selected = (char*)malloc(end - start + 1);
    strncpy(selected, b->editable_content + start, end - start);
    selected[end - start] = '\0';

    platform_set_clipboard(selected);
    free(selected);
}
