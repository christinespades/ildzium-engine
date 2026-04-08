#pragma once
#include "core/colors.h"
// each ui ctx module (ui_main, ui_sounds) includes this. all modules are in turn included in ui_modes.h
// other ui modules also use it

// UI-specific colors
#define COLOR_BORDER         COLOR_RED_33
#define COLOR_TEXT_HIGHLIGHT COLOR_BLUE_70
#define COLOR_SELECTED_TEXT  COLOR_GREEN_70
#define COLOR_BOTTOM_INFO_PANEL_BG COLOR_BLACK
#define COLOR_BOTTOM_INFO_PANEL_BORDER COLOR_YELLOW_33
#define COLOR_BOTTOM_INFO_PANEL_TEXT COLOR_WHITE
#define COLOR_EDITOR_MAIN_TEXT   COLOR_WHITE
#define COLOR_EDITOR_CARET       COLOR_MAGENTA_70
#define COLOR_EDITOR_LINE_NUMBERS COLOR_ORANGE_70
#define COLOR_MAIN_TEXT   COLOR_WHITE
#define COLOR_SCROLLBAR_BG    COLOR_RED_70
#define COLOR_SCROLLBAR_THUMB COLOR_WHITE
#define COLOR_TUNER_PANEL_IDLE     COLOR_BLACK
#define COLOR_TUNER_PANEL_ACTIVE   COLOR_BLACK_70
#define COLOR_TUNER_SPLIT_LINE COLOR_RED_70
#define COLOR_TUNER_VALUE_TEXT COLOR_WHITE

static const float BOTTOM_INFO_PANEL_AUTOSCROLL_SPEED = 30.0f;  // pixels per second (auto-scroll)

#define UI_CONTAINER_PADDING 70 // used for the main box inside each ui_ctx

#define EDITOR_HEIGHT 1400
#define EDITOR_LINE_NUMBERS_WIDTH 50
#define EDITOR_UNDO_HISTORY_AMOUNT 512
#define EDITOR_WIDTH 800
#define FONT_WIDTH   8
#define FONT_HEIGHT  8
#define MAX_BUTTONS 64