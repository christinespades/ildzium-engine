#include "pch.h"
#include "physics.h"

static void init_collision_bodies() {
    g_comp_capsule_count = 1;

    Capsule* cap = &g_comp_capsule[0];

    cap->position_radius = (vec4){
        0.0f,  // x
        100.0f, // y
        0.0f,  // z
        20.0f  // radius
    };

    cap->rotation = (vec4){
        0.0f, // x
        0.0f, // y
        0.0f, // z
        1.0f  // w
    };

    cap->height = 100.0f;
}

void physics_init() {
	init_collision_bodies();
}