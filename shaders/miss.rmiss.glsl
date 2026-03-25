// miss.rmiss.glsl
#version 460
#extension GL_EXT_ray_tracing : require
layout(location = 0) rayPayloadInEXT vec3 payload;
layout(location = 1) rayPayloadEXT float shadowPayload;
layout(binding = 2) buffer Materials { vec4 color[]; } materials;

void main() {
    payload = vec3(0.1, 0.1, 0.2); // Sky
    shadowPayload = 1.0; // No occlusion
}