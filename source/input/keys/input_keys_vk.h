#pragma once
#include "input/keys/input_keys.h"

void handle_key_input(int key, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void character_callback(GLFWwindow* window, unsigned int codepoint);
void set_key_callbacks();