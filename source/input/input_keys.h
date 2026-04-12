#pragma once
#include "core/io.h"
#include "core/window.h"
#include "input/input_keys_editor.h"
#include "input/input_keys_mods.h"
#include "scene/camera.h"
#include "ui/ui.h"
#include "ui/ui_editor.h"
    
extern UI_Context* g_ui_ctx;
void set_key_callbacks();
#ifndef __EMSCRIPTEN__
    void handle_key_input(int key, int action, int mods);
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void character_callback(GLFWwindow* window, unsigned int codepoint);
#endif