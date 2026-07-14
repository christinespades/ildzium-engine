#pragma once

#define UI_BOTTOM_INFO_PANEL_AUTOSCROLL_SPEED 1.0f // pixels per second
#define UI_COLOR_BORDER_FALLBACK COLOR_RED_33
#define UI_COLOR_BOTTOM_INFO_PANEL_BG COLOR_BLACK
#define UI_COLOR_BOTTOM_INFO_PANEL_BORDER COLOR_YELLOW_33
#define UI_COLOR_BOTTOM_INFO_PANEL_TEXT COLOR_RGB_DYNAMIC
#define UI_COLOR_CHECKBOX_BG COLOR_BLACK
#define UI_COLOR_CHECKBOX_OFF COLOR_RED_33
#define UI_COLOR_CHECKBOX_ON COLOR_GREEN_33
#define UI_COLOR_DEFAULT_BORDER_INNER COLOR_WHITE_33
#define UI_COLOR_DEFAULT_BORDER_OUTER COLOR_YELLOW_33
#define UI_COLOR_EDIT_BORDER_INNER COLOR_WHITE_33
#define UI_COLOR_EDIT_BORDER_OUTER COLOR_GREEN_33
#define UI_COLOR_EDITOR_CARET COLOR_MAGENTA_70
#define UI_COLOR_EDITOR_LINE_NUMBERS COLOR_ORANGE_70
#define UI_COLOR_EDITOR_MAIN_TEXT COLOR_WHITE
#define UI_COLOR_MAIN_TEXT COLOR_WHITE
#define UI_COLOR_SCROLL_BORDER_INNER COLOR_WHITE_33
#define UI_COLOR_SCROLL_BORDER_OUTER COLOR_MAGENTA_33
#define UI_COLOR_SCROLLBAR_BG COLOR_RED_70
#define UI_COLOR_SCROLLBAR_THUMB COLOR_WHITE
#define UI_COLOR_SCROLLBAR_THUMB_BORDER COLOR_ORANGE_33
#define UI_COLOR_SCROLLBAR_TRACK_BORDER COLOR_BLUE_33
#define UI_COLOR_SELECTED_TEXT  COLOR_GREEN_70
#define UI_COLOR_TEXT_HIGHLIGHT COLOR_BLUE_70
#define UI_COLOR_TUNER_BORDER_INNER COLOR_WHITE_33
#define UI_COLOR_TUNER_BORDER_OUTER COLOR_CYAN_33
#define UI_COLOR_TUNER_PANEL_ACTIVE   COLOR_BLACK_70
#define UI_COLOR_TUNER_PANEL_IDLE     COLOR_BLACK
#define UI_COLOR_TUNER_SPLIT_LINE COLOR_WHITE_33
#define UI_COLOR_TUNER_SPLIT_LINE_BORDER COLOR_MAGENTA_70
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
#define UI_POPUP_WIDTH 480
#define UI_POPUP_HEIGHT 320
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
#define UI_TUNER_SPLIT_LINE_THICKNESS 6.0f
#define UI_TUNER_WIDTH 300
#define UI_TUNER_HEIGHT 50

#define UI_PARAMS_MAP \
    X(f, UI_BOTTOM_INFO_PANEL_AUTOSCROLL_SPEED, "Bottom Info Panel Autoscroll Speed", -500.0f, 500.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_BORDER_FALLBACK, "Border Fallback Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_BOTTOM_INFO_PANEL_BG, "Bottom Info BG Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_BOTTOM_INFO_PANEL_BORDER, "Bottom Info Border Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_BOTTOM_INFO_PANEL_TEXT, "Bottom Info Text Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_CHECKBOX_BG, "Checkbox BG Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_CHECKBOX_ON, "Checkbox On Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_CHECKBOX_OFF, "Checkbox Off Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_DEFAULT_BORDER_INNER, "Default Border Inner Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_DEFAULT_BORDER_OUTER, "Default Border Outer Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_EDIT_BORDER_INNER, "Edit Border Inner Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_EDIT_BORDER_OUTER, "Edit Border Outer Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_EDITOR_CARET, "Editor Caret Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_EDITOR_LINE_NUMBERS, "Editor Line Numbers Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_EDITOR_MAIN_TEXT, "Editor Main Text Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_MAIN_TEXT, "Main Text Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_SCROLL_BORDER_INNER, "Scroll Border Inner Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_SCROLL_BORDER_OUTER, "Scroll Border Outer Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_SCROLLBAR_BG, "Scrollbar BG Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_SCROLLBAR_THUMB, "Scrollbar Thumb Color", 0.0f, 1.0f, tooltip_EMPTY) \
    X(u, UI_COLOR_SCROLLBAR_THUMB_BORDER, "Scrollbar Thumb Border Color", 0, 255, tooltip_EMPTY) \
    X(u, UI_COLOR_SCROLLBAR_TRACK_BORDER, "Scrollbar Track Border Color", 0, 255, tooltip_EMPTY) \
    X(u, UI_COLOR_SELECTED_TEXT, "Selected Text Color", 0, 255, tooltip_EMPTY) \
    X(u, UI_COLOR_TEXT_HIGHLIGHT, "Text Highlight Color", 0, 255, tooltip_EMPTY) \
    X(u, UI_COLOR_TUNER_BORDER_INNER, "Tuner Border Inner Color",0, 255, tooltip_EMPTY) \
    X(u, UI_COLOR_TUNER_BORDER_OUTER, "Tuner Border Outer Color", 0, 255, tooltip_EMPTY) \
    X(u, UI_COLOR_TUNER_PANEL_ACTIVE, "Tuner Panel Active Color", 0, 255, tooltip_EMPTY) \
    X(u, UI_COLOR_TUNER_PANEL_IDLE, "Tuner Panel Idle Color", 0, 1, tooltip_EMPTY) \
    X(u, UI_COLOR_TUNER_SPLIT_LINE, "Tuner Split Line Color", 0, 255, tooltip_EMPTY) \
    X(u, UI_COLOR_TUNER_SPLIT_LINE_BORDER, "Tuner Split Line Color (Border)", 0, 255, tooltip_EMPTY) \
    X(u, UI_COLOR_TUNER_VALUE_TEXT, "Tuner Value Text Color", 0, 255, tooltip_EMPTY) \
    X(f, UI_CONTAINER_PADDING, "Container Padding", 0.0f, 2000.0f, tooltip_EMPTY) \
    X(f, UI_EDITOR_HEIGHT, "Editor Height", 10.0f, 2160.0f, tooltip_EMPTY) \
    X(f, UI_EDITOR_LINE_NUMBERS_WIDTH, "Editor Line Numbers Width", 0.0f, 500.0f, tooltip_EMPTY) \
    X(f, UI_EDITOR_UNDO_HISTORY_AMOUNT, "Editor Undo History Amount", 1.0f, 2048.0f, tooltip_EMPTY) \
    X(f, UI_EDITOR_WIDTH, "Editor Width", 10.0f, 3840.0f, tooltip_EMPTY) \
    X(f, UI_ELEMENT_START_X, "Element Start X", 0.0f, 3840.0f, tooltip_EMPTY) \
    X(f, UI_ELEMENT_START_Y, "Element Start Y", 0.0f, 3840.0f, tooltip_EMPTY) \
    X(f, UI_FONT_HEIGHT, "Font Height", 1.0f, 128.0f, tooltip_EMPTY) \
    X(f, UI_FONT_WIDTH, "Font Width", 1.0f, 128.0f, tooltip_EMPTY) \
    X(f, UI_MAX_BUTTONS, "Max Buttons Limit", 64.0f, 128.0f, tooltip_EMPTY) \
    X(f, UI_POPUP_HEIGHT, "Popup Height", 100.0f, 1280.0f, tooltip_EMPTY) \
    X(f, UI_POPUP_WIDTH, "Popup Width", 100.0f, 1280.0f, tooltip_EMPTY) \
    X(f, UI_SCROLLBAR_V_THICKNESS, "Scrollbar (Vertical) Thickness", 0.0f, 64.0f, tooltip_EMPTY) \
    X(f, UI_SCROLLBAR_V_PAD_LEFT, "Scrollbar (Vertical) Padding Left", 0.0f, 64.0f, tooltip_EMPTY) \
    X(f, UI_SCROLLBAR_V_PAD_RIGHT, "Scrollbar (Vertical) Padding Right", 0.0f, 64.0f, tooltip_EMPTY) \
    X(f, UI_SCROLLBAR_V_PAD_TOP, "Scrollbar (Vertical) Padding Top", 0.0f, 64.0f, tooltip_EMPTY) \
    X(f, UI_SCROLLBAR_V_PAD_BOTTOM, "Scrollbar (Vertical) Padding Bottom", 0.0f, 64.0f, tooltip_EMPTY) \
    X(f, UI_SCROLLBAR_H_THICKNESS, "Scrollbar (Horizontal) Thickness", 0.0f, 64.0f, tooltip_EMPTY) \
    X(f, UI_SCROLLBAR_H_PAD_LEFT, "Scrollbar (Horizontal) Padding Left", 0.0f, 64.0f, tooltip_EMPTY) \
    X(f, UI_SCROLLBAR_H_PAD_RIGHT, "Scrollbar (Horizontal) Padding Right", 0.0f, 64.0f, tooltip_EMPTY) \
    X(f, UI_SCROLLBAR_H_PAD_TOP, "Scrollbar (Horizontal) Padding Top", 0.0f, 64.0f, tooltip_EMPTY) \
    X(f, UI_SCROLLBAR_H_PAD_BOTTOM, "Scrollbar (Horizontal) Padding Bottom", 0.0f, 64.0f, tooltip_EMPTY) \
    X(f, UI_TUNER_SPLIT_LINE_THICKNESS, "Tuner Split Line Thickness", 1.0f, 50.0f, tooltip_EMPTY) \
    X(f, UI_TUNER_WIDTH, "Tuner Width", 140.0f, 400.0f, tooltip_EMPTY) \
    X(f, UI_TUNER_HEIGHT, "Tuner Height", 40.0f, 90.0f, tooltip_EMPTY)
