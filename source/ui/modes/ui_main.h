#pragma once
#include "core/http.h"
#include "core/string.h"
#include "core/parser_md.h"
#include "ui/ui.h"
#include "ui/ui_elements.h"
#include "ui/ui_params.h"
#include "ui/ui_draw.h"
#include "ui/ui_markdown.h"

static void on_readme_loaded(char* readme, void* user);
void setup_main_controls(UI_Context* ctx);