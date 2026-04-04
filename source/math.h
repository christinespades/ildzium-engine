#pragma once
#include <stdbool.h>   // for bool type
#include <string.h>    // if you use memset elsewhere

bool matrix_inverse(const float m[16], float invOut[16]);