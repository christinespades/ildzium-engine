#pragma once
#include "core/settings.h"
#include "core/time.h"
#include "core/tooltips.h"
#include "ui/ui_context.h"
#include "input/input.h"
#include "scene/sky.h"
#include "ui/ui_button.h"
#include "ui/ui_callbacks.h"
#include "ui/ui_editor.h"
#include "ui/ui_text.h"
#include "ui/ui_mode.h"
#include "ui/ui_update.h"
#include "rendering/fps.h"

void ui_init(UI_Context* ctx);
void ui_cleanup(UI_Context* ctx);   // important for freeing memory