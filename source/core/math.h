#pragma once

void matrix_multiply(const float* a, const float* b, float* out);   // out = a * b
void matrix_identity(float* out);
bool matrix_inverse(const float m[16], float invOut[16]);
void matrix_scale(float m[16], float sx, float sy, float sz);
void matrix_make_scale(float m[16], float sx, float sy, float sz);