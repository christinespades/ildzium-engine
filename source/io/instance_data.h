#pragma once

typedef struct {
    float model[16];     // mat4 - 64 bytes
    float color[4];      // vec4 - 16 bytes
} InstanceData; // Perfectly matches old 80-byte shader alignment
