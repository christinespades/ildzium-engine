#pragma once

typedef struct Capsule
{
    vec4 position_radius;
    vec4 rotation;
    float height;
} Capsule;

#define MAX_CAPSULES 4096

extern Capsule g_comp_capsule[MAX_CAPSULES];
extern uint32_t g_comp_capsule_count;