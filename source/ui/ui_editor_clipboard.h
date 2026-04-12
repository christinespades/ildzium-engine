#pragma once
#include "ui/ui_editor_string.h"

#ifndef __EMSCRIPTEN__
	#include "core/window.h"
#endif
	void platform_set_clipboard(const char* text);
	void platform_get_clipboard_async(void (*callback)(const char*, void*), void* user);
	static void paste_callback(const char* text, void* user);
	void paste_from_clipboard(UI_Button* b);
	void copy_selection_to_clipboard(UI_Button* b);