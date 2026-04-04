#include "math.h"

bool matrix_inverse(const float m[16], float invOut[16])
{
    float inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] - m[5]  * m[11] * m[14] - m[9]  * m[6]  * m[15] +
             m[9]  * m[7]  * m[14] + m[13] * m[6]  * m[11] - m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + m[4]  * m[11] * m[14] + m[8]  * m[6]  * m[15] -
             m[8]  * m[7]  * m[14] - m[12] * m[6]  * m[11] + m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - m[4]  * m[11] * m[13] - m[8]  * m[5] * m[15] +
             m[8]  * m[7]  * m[13] + m[12] * m[5]  * m[11] - m[12] * m[7]  * m[9];

    inv[12] = -m[4] * m[9] * m[14] + m[4]  * m[10] * m[13] + m[8]  * m[5] * m[14] -
              m[8]  * m[6]  * m[13] - m[12] * m[5]  * m[10] + m[12] * m[6]  * m[9];

    inv[1] = -m[1] * m[10] * m[15] + m[1]  * m[11] * m[14] + m[9]  * m[2] * m[15] -
             m[9]  * m[3]  * m[14] - m[13] * m[2]  * m[11] + m[13] * m[3]  * m[10];

    inv[5] = m[0]  * m[10] * m[15] - m[0]  * m[11] * m[14] - m[8]  * m[2] * m[15] +
             m[8]  * m[3]  * m[14] + m[12] * m[2]  * m[11] - m[12] * m[3]  * m[10];

    inv[9] = -m[0] * m[9] * m[15] + m[0]  * m[11] * m[13] + m[8]  * m[1] * m[15] -
             m[8]  * m[3]  * m[13] - m[12] * m[1]  * m[11] + m[12] * m[3]  * m[9];

    inv[13] = m[0] * m[9] * m[14] - m[0]  * m[10] * m[13] - m[8]  * m[1] * m[14] +
              m[8]  * m[2]  * m[13] + m[12] * m[1]  * m[10] - m[12] * m[2]  * m[9];

    inv[2] = m[1]  * m[6] * m[15] - m[1]  * m[7] * m[14] - m[5] * m[2] * m[15] +
             m[5]  * m[3] * m[14] + m[13] * m[2] * m[7]  - m[13] * m[3] * m[6];

    inv[6] = -m[0] * m[6] * m[15] + m[0]  * m[7] * m[14] + m[4] * m[2] * m[15] -
             m[4]  * m[3] * m[14] - m[12] * m[2] * m[7]  + m[12] * m[3] * m[6];

    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
              m[4]  * m[3] * m[13] + m[12] * m[1] * m[7]  - m[12] * m[3] * m[5];

    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] -
              m[4]  * m[2] * m[13] - m[12] * m[1] * m[6]  + m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] -
             m[5]  * m[3] * m[10] - m[9]  * m[2] * m[7]  + m[9]  * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] +
             m[4]  * m[3] * m[10] + m[8]  * m[2] * m[7]  - m[8]  * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] -
              m[4]  * m[3] * m[9]  - m[8]  * m[1] * m[7]  + m[8]  * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] +
              m[4]  * m[2] * m[9]  + m[8]  * m[1] * m[6]  - m[8]  * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0.0f) {
        return false;  // singular matrix
    }

    det = 1.0f / det;

    for (i = 0; i < 16; i++) {
        invOut[i] = inv[i] * det;
    }

    return true;
}

void matrix_identity(float m[16]);

void matrix_scale(float m[16], float sx, float sy, float sz)
{
    m[0]  *= sx;  m[1]  *= sx;  m[2]  *= sx;  m[3]  *= sx;
    m[4]  *= sy;  m[5]  *= sy;  m[6]  *= sy;  m[7]  *= sy;
    m[8]  *= sz;  m[9]  *= sz;  m[10] *= sz;  m[11] *= sz;
    // translation (elements 12,13,14) unchanged
}

// If you prefer a version that starts from identity:
void matrix_make_scale(float m[16], float sx, float sy, float sz)
{
    matrix_identity(m);
    matrix_scale(m, sx, sy, sz);
}