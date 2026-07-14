#pragma once

typedef struct {
    float x, y, z;
} vec3;

typedef struct {
    float x, y, z, w;
} vec4;

#include "core/math/math_float.h"
#include "core/math/math_matrix.h"
#include "core/math/math_random.h"
#include "core/math/math_vector.h"
#include "core/math/math_quat.h"

#define DEG2RAD(x) ((x) * (PI / 180.0f))