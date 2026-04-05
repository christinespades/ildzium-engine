#pragma once
#include "ui/ui.h"
#include "ui/ui_draw.h"

static size_t write_to_string(void *ptr, size_t size, size_t nmemb, void *userp);
char* fetch_readme(const char* url);
void setup_main_menu_controls(UI_Context* ctx);