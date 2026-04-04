#pragma once
#include <stdbool.h>   // for bool type
#include <string.h>    // if you use memset elsewhere

bool matrix_inverse(const float m[16], float invOut[16]);
void matrix_identity(float m[16]);
void matrix_scale(float m[16], float sx, float sy, float sz);
void matrix_make_scale(float m[16], float sx, float sy, float sz);