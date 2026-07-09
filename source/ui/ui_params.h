#pragma once
#include "core/colors.h"
// each ui ctx module (ui_main, ui_sounds) includes this. all modules are in turn included in ui_modes.h
// other ui modules also use it

static const float UI_BOTTOM_INFO_PANEL_AUTOSCROLL_SPEED = 30.0f;  // pixels per second (auto-scroll)
#define UI_COLOR_BORDER_FALLBACK         COLOR_RED_33
#define UI_COLOR_BOTTOM_INFO_PANEL_BG COLOR_BLACK
#define UI_COLOR_BOTTOM_INFO_PANEL_BORDER COLOR_YELLOW_33
#define UI_COLOR_BOTTOM_INFO_PANEL_TEXT COLOR_RGB_DYNAMIC
#define UI_COLOR_DEFAULT_BORDER_INNER COLOR_WHITE_33
#define UI_COLOR_DEFAULT_BORDER_OUTER COLOR_YELLOW_33
#define UI_COLOR_EDIT_BORDER_INNER COLOR_WHITE_33
#define UI_COLOR_EDIT_BORDER_OUTER COLOR_GREEN_33
#define UI_COLOR_EDITOR_CARET       COLOR_MAGENTA_70
#define UI_COLOR_EDITOR_LINE_NUMBERS COLOR_ORANGE_70
#define UI_COLOR_EDITOR_MAIN_TEXT   COLOR_WHITE
#define UI_COLOR_MAIN_TEXT   COLOR_WHITE
#define UI_COLOR_SCROLL_BORDER_INNER COLOR_WHITE_33
#define UI_COLOR_SCROLL_BORDER_OUTER COLOR_MAGENTA_33
#define UI_COLOR_SCROLLBAR_BG    COLOR_RED_70
#define UI_COLOR_SCROLLBAR_THUMB COLOR_WHITE
#define UI_COLOR_SCROLLBAR_THUMB_BORDER COLOR_ORANGE_33
#define UI_COLOR_SCROLLBAR_TRACK_BORDER COLOR_BLUE_33
#define UI_COLOR_SELECTED_TEXT  COLOR_GREEN_70
#define UI_COLOR_TEXT_HIGHLIGHT COLOR_BLUE_70
#define UI_COLOR_TUNER_BORDER_INNER COLOR_WHITE_33
#define UI_COLOR_TUNER_BORDER_OUTER COLOR_CYAN_33
#define UI_COLOR_TUNER_PANEL_ACTIVE   COLOR_BLACK_70
#define UI_COLOR_TUNER_PANEL_IDLE     COLOR_BLACK
#define UI_COLOR_TUNER_SPLIT_LINE COLOR_RED_70
#define UI_COLOR_TUNER_VALUE_TEXT COLOR_WHITE
#define UI_CONTAINER_PADDING 70 // used for the main box inside each ui_ctx
#define UI_EDITOR_HEIGHT 1400
#define UI_EDITOR_LINE_NUMBERS_WIDTH 50
#define UI_EDITOR_UNDO_HISTORY_AMOUNT 512
#define UI_EDITOR_WIDTH 800
#define UI_ELEMENT_START_X 10
#define UI_ELEMENT_START_Y 70
#define UI_FONT_HEIGHT  8
#define UI_FONT_WIDTH   8
#define UI_MAX_BUTTONS 128
#define UI_POPUP_WIDTH 1080
#define UI_POPUP_HEIGHT 720
#define UI_SCROLLBAR_V_THICKNESS 10
#define UI_SCROLLBAR_V_PAD_LEFT 12
#define UI_SCROLLBAR_V_PAD_RIGHT 12
#define UI_SCROLLBAR_V_PAD_TOP 6
#define UI_SCROLLBAR_V_PAD_BOTTOM 18
#define UI_SCROLLBAR_H_THICKNESS 10
#define UI_SCROLLBAR_H_PAD_LEFT 6
#define UI_SCROLLBAR_H_PAD_RIGHT 18
#define UI_SCROLLBAR_H_PAD_TOP 12
#define UI_SCROLLBAR_H_PAD_BOTTOM 12
#define UI_TUNER_WIDTH 300
#define UI_TUNER_HEIGHT 50

#define UI_PARAMS_MAP \
    X(f, UI_BOTTOM_INFO_PANEL_AUTOSCROLL_SPEED,   UI_BOTTOM_INFO_PANEL_AUTOSCROLL_SPEED, "Bottom Info Panel Autoscroll Speed", -500.0f, 500.0f) \
    X(u, UI_COLOR_BORDER_FALLBACK,               UI_COLOR_BORDER_FALLBACK,               "Border Fallback Color",              0.0f, 4294967295.0f) \
    X(u, UI_COLOR_BOTTOM_INFO_PANEL_BG,           UI_COLOR_BOTTOM_INFO_PANEL_BG,           "Bottom Info BG Color",               0.0f, 4294967295.0f) \
    X(u, UI_COLOR_BOTTOM_INFO_PANEL_BORDER,       UI_COLOR_BOTTOM_INFO_PANEL_BORDER,       "Bottom Info Border Color",           0.0f, 4294967295.0f) \
    X(u, UI_COLOR_BOTTOM_INFO_PANEL_TEXT,         UI_COLOR_BOTTOM_INFO_PANEL_TEXT,         "Bottom Info Text Color",             0.0f, 4294967295.0f) \
    X(u, UI_COLOR_DEFAULT_BORDER_INNER,           UI_COLOR_DEFAULT_BORDER_INNER,           "Default Border Inner Color",         0.0f, 4294967295.0f) \
    X(u, UI_COLOR_DEFAULT_BORDER_OUTER,           UI_COLOR_DEFAULT_BORDER_OUTER,           "Default Border Outer Color",         0.0f, 4294967295.0f) \
    X(u, UI_COLOR_EDIT_BORDER_INNER,              UI_COLOR_EDIT_BORDER_INNER,              "Edit Border Inner Color",            0.0f, 4294967295.0f) \
    X(u, UI_COLOR_EDIT_BORDER_OUTER,              UI_COLOR_EDIT_BORDER_OUTER,              "Edit Border Outer Color",            0.0f, 4294967295.0f) \
    X(u, UI_COLOR_EDITOR_CARET,                   UI_COLOR_EDITOR_CARET,                   "Editor Caret Color",                 0.0f, 4294967295.0f) \
    X(u, UI_COLOR_EDITOR_LINE_NUMBERS,            UI_COLOR_EDITOR_LINE_NUMBERS,            "Editor Line Numbers Color",          0.0f, 4294967295.0f) \
    X(u, UI_COLOR_EDITOR_MAIN_TEXT,               UI_COLOR_EDITOR_MAIN_TEXT,               "Editor Main Text Color",             0.0f, 4294967295.0f) \
    X(u, UI_COLOR_MAIN_TEXT,                      UI_COLOR_MAIN_TEXT,                      "Main Text Color",                    0.0f, 4294967295.0f) \
    X(u, UI_COLOR_SCROLL_BORDER_INNER,            UI_COLOR_SCROLL_BORDER_INNER,            "Scroll Border Inner Color",          0.0f, 4294967295.0f) \
    X(u, UI_COLOR_SCROLL_BORDER_OUTER,            UI_COLOR_SCROLL_BORDER_OUTER,            "Scroll Border Outer Color",          0.0f, 4294967295.0f) \
    X(u, UI_COLOR_SCROLLBAR_BG,                   UI_COLOR_SCROLLBAR_BG,                   "Scrollbar BG Color",                 0.0f, 4294967295.0f) \
    X(u, UI_COLOR_SCROLLBAR_THUMB,                UI_COLOR_SCROLLBAR_THUMB,                "Scrollbar Thumb Color",              0.0f, 4294967295.0f) \
    X(u, UI_COLOR_SCROLLBAR_THUMB_BORDER,         UI_COLOR_SCROLLBAR_THUMB_BORDER,         "Scrollbar Thumb Border Color",       0.0f, 4294967295.0f) \
    X(u, UI_COLOR_SCROLLBAR_TRACK_BORDER,         UI_COLOR_SCROLLBAR_TRACK_BORDER,         "Scrollbar Track Border Color",       0.0f, 4294967295.0f) \
    X(u, UI_COLOR_SELECTED_TEXT,                  UI_COLOR_SELECTED_TEXT,                  "Selected Text Color",                0.0f, 4294967295.0f) \
    X(u, UI_COLOR_TEXT_HIGHLIGHT,                 UI_COLOR_TEXT_HIGHLIGHT,                 "Text Highlight Color",               0.0f, 4294967295.0f) \
    X(u, UI_COLOR_TUNER_BORDER_INNER,             UI_COLOR_TUNER_BORDER_INNER,             "Tuner Border Inner Color",           0.0f, 4294967295.0f) \
    X(u, UI_COLOR_TUNER_BORDER_OUTER,             UI_COLOR_TUNER_BORDER_OUTER,             "Tuner Border Outer Color",           0.0f, 4294967295.0f) \
    X(u, UI_COLOR_TUNER_PANEL_ACTIVE,             UI_COLOR_TUNER_PANEL_ACTIVE,             "Tuner Panel Active Color",           0.0f, 4294967295.0f) \
    X(u, UI_COLOR_TUNER_PANEL_IDLE,               UI_COLOR_TUNER_PANEL_IDLE,               "Tuner Panel Idle Color",             0.0f, 4294967295.0f) \
    X(u, UI_COLOR_TUNER_SPLIT_LINE,               UI_COLOR_TUNER_SPLIT_LINE,               "Tuner Split Line Color",             0.0f, 4294967295.0f) \
    X(u, UI_COLOR_TUNER_VALUE_TEXT,               UI_COLOR_TUNER_VALUE_TEXT,               "Tuner Value Text Color",             0.0f, 4294967295.0f) \
    X(f, UI_CONTAINER_PADDING,                    UI_CONTAINER_PADDING,                    "Container Padding",                  0.0f, 2000.0f) \
    X(f, UI_EDITOR_HEIGHT,                        UI_EDITOR_HEIGHT,                        "Editor Height",                     10.0f, 2160.0f) \
    X(f, UI_EDITOR_LINE_NUMBERS_WIDTH,            UI_EDITOR_LINE_NUMBERS_WIDTH,            "Editor Line Numbers Width",          0.0f, 500.0f) \
    X(f, UI_EDITOR_UNDO_HISTORY_AMOUNT,           UI_EDITOR_UNDO_HISTORY_AMOUNT,           "Editor Undo History Amount",         1.0f, 2048.0f) \
    X(f, UI_EDITOR_WIDTH,                         UI_EDITOR_WIDTH,                         "Editor Width",                      10.0f, 3840.0f) \
    X(f, UI_ELEMENT_START_X,                      UI_ELEMENT_START_X,                      "Element Start X",                    0.0f, 3840.0f) \
    X(f, UI_ELEMENT_START_Y,                      UI_ELEMENT_START_Y,                      "Element Start Y",                    0.0f, 3840.0f) \
    X(f, UI_FONT_HEIGHT,                          UI_FONT_HEIGHT,                          "Font Height",                        1.0f, 128.0f) \
    X(f, UI_FONT_WIDTH,                           UI_FONT_WIDTH,                           "Font Width",                         1.0f, 128.0f) \
    X(f, UI_MAX_BUTTONS,                          UI_MAX_BUTTONS,                          "Max Buttons Limit",                 64.0f, 128.0f) \
    X(f, UI_POPUP_HEIGHT,                           UI_POPUP_HEIGHT,                           "Popup Height",                         100.0f, 1280.0f) \
    X(f, UI_POPUP_WIDTH,                          UI_POPUP_WIDTH,                          "Popup Width",                 100.0f, 1280.0f) \
    X(f, UI_SCROLLBAR_V_THICKNESS,   UI_SCROLLBAR_V_THICKNESS,   "Scrollbar (Vertical) Thickness",   0.0f, 64.0f) \
    X(f, UI_SCROLLBAR_V_PAD_LEFT,   UI_SCROLLBAR_V_PAD_LEFT,   "Scrollbar (Vertical) Padding Left",   0.0f, 64.0f) \
    X(f, UI_SCROLLBAR_V_PAD_RIGHT,  UI_SCROLLBAR_V_PAD_RIGHT,  "Scrollbar (Vertical) Padding Right",  0.0f, 64.0f) \
    X(f, UI_SCROLLBAR_V_PAD_TOP,    UI_SCROLLBAR_V_PAD_TOP,    "Scrollbar (Vertical) Padding Top",    0.0f, 64.0f) \
    X(f, UI_SCROLLBAR_V_PAD_BOTTOM, UI_SCROLLBAR_V_PAD_BOTTOM, "Scrollbar (Vertical) Padding Bottom", 0.0f, 64.0f) \
    X(f, UI_SCROLLBAR_H_THICKNESS,   UI_SCROLLBAR_H_THICKNESS,   "Scrollbar (Horizontal) Thickness",   0.0f, 64.0f) \
    X(f, UI_SCROLLBAR_H_PAD_LEFT,   UI_SCROLLBAR_H_PAD_LEFT,   "Scrollbar (Horizontal) Padding Left",   0.0f, 64.0f) \
    X(f, UI_SCROLLBAR_H_PAD_RIGHT,  UI_SCROLLBAR_H_PAD_RIGHT,  "Scrollbar (Horizontal) Padding Right",  0.0f, 64.0f) \
    X(f, UI_SCROLLBAR_H_PAD_TOP,    UI_SCROLLBAR_H_PAD_TOP,    "Scrollbar (Horizontal) Padding Top",    0.0f, 64.0f) \
    X(f, UI_SCROLLBAR_H_PAD_BOTTOM, UI_SCROLLBAR_H_PAD_BOTTOM, "Scrollbar (Horizontal) Padding Bottom", 0.0f, 64.0f) \
    X(f, UI_TUNER_WIDTH,                           UI_TUNER_WIDTH,                           "Tuner Width",                       140.0f, 400.0f) \
    X(f, UI_TUNER_HEIGHT,                          UI_TUNER_HEIGHT,                          "Tuner Height",                      40.0f, 90.0f)

typedef enum {
    #define X(type, id, fallback, name_str, min, max) PARAM_##id,
    UI_PARAMS_MAP
    #undef X
    PARAM_COUNT
} MacroParamID;

// Internal storage array for live-tracked parameter variables
typedef union {
    float f;
    uint32_t u;
} UI_ParamValue;

// Update your global storage pool to use this new variant array
extern UI_ParamValue s_macro_param_registry[PARAM_COUNT];
static bool s_macro_registry_initialized = false;

void init_macro_param_registry(void);

static inline float get_param_float(MacroParamID param_id) {
    return s_macro_param_registry[param_id].f;
}

static inline uint32_t get_param_color(MacroParamID param_id) {
    // Return the clean, unmutated 32-bit integer color sequence
    return s_macro_param_registry[param_id].u;
}