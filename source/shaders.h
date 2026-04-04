#pragma once
#include <stdint.h> 

uint32_t* load_spirv(const char* filename, size_t* out_size);
void on_shader_changed(const char* path);