#pragma once
#include <GLFW/glfw3.h>

int mouse_wheel;
void init_input();
void handle_text_input(unsigned int codepoint);   // new
void handle_key_input(int key, int action, int mods); // if you want to centralize
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);