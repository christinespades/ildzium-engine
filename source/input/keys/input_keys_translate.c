#include "pch.h"
#include "input/keys/input_keys_translate.h"

platform_key translate_code(const char* code)
{
    // Letters
    if (strncmp(code, "Key", 3) == 0) {
        char c = code[3];
        if (c >= 'A' && c <= 'Z') {
            return (platform_key)(KEY_A + (c - 'A'));
        }
    }

    // Digits
    if (strncmp(code, "Digit", 5) == 0) {
        char c = code[5];
        if (c >= '0' && c <= '9') {
            return (platform_key)(KEY_0 + (c - '0'));
        }
    }

    // Function keys
	if (code[0] == 'F') {
	    int num = 0;
	    if (code[1] >= '0' && code[1] <= '9') {
	        num = code[1] - '0';
	        if (code[2] >= '0' && code[2] <= '9') {
	            num = num * 10 + (code[2] - '0');
	        }
	        if (num >= 1 && num <= 25) {
	            return (platform_key)(KEY_F1 + (num - 1));
	        }
	    }
	}

    // Arrows
    if (strcmp(code, "ArrowLeft") == 0)  return KEY_LEFT;
    if (strcmp(code, "ArrowRight") == 0) return KEY_RIGHT;
    if (strcmp(code, "ArrowUp") == 0)    return KEY_UP;
    if (strcmp(code, "ArrowDown") == 0)  return KEY_DOWN;

    // Modifiers (full L/R support)
    if (strcmp(code, "ShiftLeft") == 0)   return KEY_LEFT_SHIFT;
    if (strcmp(code, "ShiftRight") == 0)  return KEY_RIGHT_SHIFT;
    if (strcmp(code, "ControlLeft") == 0) return KEY_LEFT_CONTROL;
    if (strcmp(code, "ControlRight") == 0)return KEY_RIGHT_CONTROL;
    if (strcmp(code, "AltLeft") == 0)     return KEY_LEFT_ALT;
    if (strcmp(code, "AltRight") == 0)    return KEY_RIGHT_ALT;

    // System keys
    if (strcmp(code, "Escape") == 0)      return KEY_ESCAPE;
    if (strcmp(code, "Enter") == 0)       return KEY_ENTER;
    if (strcmp(code, "Tab") == 0)         return KEY_TAB;
    if (strcmp(code, "Backspace") == 0)   return KEY_BACKSPACE;
    if (strcmp(code, "Delete") == 0)      return KEY_DELETE;
    if (strcmp(code, "Insert") == 0)      return KEY_INSERT;
    if (strcmp(code, "Home") == 0)        return KEY_HOME;
    if (strcmp(code, "End") == 0)         return KEY_END;
    if (strcmp(code, "PageUp") == 0)      return KEY_PAGE_UP;
    if (strcmp(code, "PageDown") == 0)    return KEY_PAGE_DOWN;

    // Locks
    if (strcmp(code, "CapsLock") == 0)    return KEY_CAPS_LOCK;
    if (strcmp(code, "NumLock") == 0)     return KEY_NUM_LOCK;
    if (strcmp(code, "ScrollLock") == 0)  return KEY_SCROLL_LOCK;
    if (strcmp(code, "Pause") == 0)       return KEY_PAUSE;

    // Symbols
    if (strcmp(code, "Space") == 0)           return KEY_SPACE;
    if (strcmp(code, "Minus") == 0)           return KEY_MINUS;
    if (strcmp(code, "Equal") == 0)           return KEY_EQUAL;
    if (strcmp(code, "BracketLeft") == 0)     return KEY_LEFT_BRACKET;
    if (strcmp(code, "BracketRight") == 0)    return KEY_RIGHT_BRACKET;
    if (strcmp(code, "Backslash") == 0)       return KEY_BACKSLASH;
    if (strcmp(code, "Semicolon") == 0)       return KEY_SEMICOLON;
    if (strcmp(code, "Quote") == 0)           return KEY_APOSTROPHE;
    if (strcmp(code, "Backquote") == 0)       return KEY_GRAVE_ACCENT;
    if (strcmp(code, "Comma") == 0)           return KEY_COMMA;
    if (strcmp(code, "Period") == 0)          return KEY_PERIOD;
    if (strcmp(code, "Slash") == 0)           return KEY_SLASH;

	// Numpad digits
	if (strncmp(code, "Numpad", 7) == 0) {
	    char c = code[7];

	    if (c >= '0' && c <= '9') {
	        return (platform_key)(KEY_KP_0 + (c - '0'));
	    }

	    if (strcmp(code, "NumpadDecimal") == 0)  return KEY_KP_DECIMAL;
	    if (strcmp(code, "NumpadDivide") == 0)   return KEY_KP_DIVIDE;
	    if (strcmp(code, "NumpadMultiply") == 0) return KEY_KP_MULTIPLY;
	    if (strcmp(code, "NumpadSubtract") == 0) return KEY_KP_SUBTRACT;
	    if (strcmp(code, "NumpadAdd") == 0)      return KEY_KP_ADD;
	    if (strcmp(code, "NumpadEnter") == 0)    return KEY_KP_ENTER;
	    if (strcmp(code, "NumpadEqual") == 0)    return KEY_KP_EQUAL;
	}

	if (strcmp(code, "MetaLeft") == 0)  return KEY_LEFT_SUPER;
	if (strcmp(code, "MetaRight") == 0) return KEY_RIGHT_SUPER;

	if (strcmp(code, "PrintScreen") == 0) return KEY_PRINT_SCREEN;
	if (strcmp(code, "ContextMenu") == 0) return KEY_MENU;

    return KEY_UNKNOWN;
}