// anyhit.rahit.glsl
#version 460
#extension GL_EXT_ray_tracing : require
layout(location = 0) rayPayloadInEXT vec3 payload;
layout(binding = 2, std430) buffer Materials { vec4 materials[]; };

void main() {
    uint matIdx = gl_InstanceCustomIndexEXT;
    vec4 material = materials[matIdx];
    if (material.a > 0.0) { // Refractive
        payload += material.rgb * 0.5; // Tint
        ignoreIntersectionEXT; // Continue tracingasdasd
    }
}