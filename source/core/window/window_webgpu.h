#pragma once
#include "core/window/window.h"

static EM_BOOL on_resize(int eventType, const EmscriptenUiEvent* e, void* userData);
void init_window(void);
void ildz_get_window_size(int* w, int* h);