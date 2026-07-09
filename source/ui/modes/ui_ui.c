#include "pch.h"
#include "ui_ui.h"
#include "ui/ui_elements.h"
#include "ui/ui_layout.h"

void setup_ui_controls(UI_Context* ctx) 
{
    UI_Layout l = ui_layout_begin(ctx);

    // The stringify operator (#type) turns f into "f" and u into "u" automatically
    #define X(type, id, fallback, name_str, min_val, max_val) \
        ui_add_macro_tuner(ctx, l.current_x, ui_layout_next_y(&l), l.btn_w, l.btn_h, \
                           name_str, PARAM_##id, min_val, max_val, (#type[0] == 'u'));
    UI_PARAMS_MAP
    #undef X
}