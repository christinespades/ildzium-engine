#pragma once
#include "input/input_keys.h"
#include "input/input_mouse.h"

void init_input();

#ifndef __EMSCRIPTEN__
	void handle_text_input(unsigned int codepoint);
    void input_toggle_mode(GLFWwindow* window);
#else
    void input_toggle_mode();
#endif