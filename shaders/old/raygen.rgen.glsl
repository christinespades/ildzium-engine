// raygen.rgen.glsl
#version 460
#extension GL_EXT_ray_tracing : require
layout(binding = 0) uniform accelerationStructureEXT tlas;
layout(binding = 1, rgba8) uniform image2D outputImage; // Renamed from 'output'
layout(binding = 2) buffer Materials { vec4 color[]; } materials;
layout(location = 0) rayPayloadEXT vec3 payload;

layout(push_constant) uniform PushConstants {
    mat4 viewProjInv;
    vec3 cameraPos;
} pc;

void main() {
    vec2 uv = vec2(gl_LaunchIDEXT.xy) / vec2(gl_LaunchSizeEXT.xy - 1) * 2.0 - 1.0;
    vec4 clipPos = vec4(uv, 0.0, 1.0);
    vec4 worldPos = pc.viewProjInv * clipPos;
    vec3 dir = normalize(worldPos.xyz / worldPos.w - pc.cameraPos);
    payload = vec3(0.0);
    traceRayEXT(tlas, gl_RayFlagsNoneEXT, 0xFF, 0, 0, 0, pc.cameraPos, 0.01, dir, 1000.0, 0);
    imageStore(outputImage, ivec2(gl_LaunchIDEXT.xy), vec4(payload, 1.0)); // Updated reference
}