// closesthit.rchit.glsl
#version 460
#extension GL_EXT_ray_tracing : require
layout(location = 0) rayPayloadInEXT vec3 payload;
layout(location = 1) rayPayloadEXT float shadowPayload;
layout(binding = 0) uniform accelerationStructureEXT tlas;
layout(binding = 2, std430) buffer Materials { vec4 materials[]; }; // RGBA, alpha = 0 for opaque
hitAttributeEXT vec3 attribNormal; // Normal from the hit triangle

void main() {
    uint matIdx = gl_InstanceCustomIndexEXT;
    vec4 material = materials[matIdx];
    vec3 albedo = material.rgb;
    vec3 hitPos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
    vec3 normal = normalize(attribNormal); // Interpolated normal from vertex data

    // Direct lighting
    vec3 lightPos = vec3(10.0, 10.0, 10.0);
    vec3 lightDir = normalize(lightPos - hitPos);
    float shadow = 1.0;
    shadowPayload = 0.0;
    traceRayEXT(tlas, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, 0, 0, 1, hitPos, 0.01, lightDir, length(lightPos - hitPos), 1);
    if (shadowPayload > 0.0) shadow = 0.0;

    // GI (simple diffuse)
    vec3 giDir = normalize(normal + vec3(0.5, 0.5, 0.5)); // Hacky hemisphere sample
    vec3 gi = vec3(0.0);
    traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, hitPos, 0.01, giDir, 1000.0, 0);
    gi = payload * 0.2;

    payload = albedo * (max(dot(normal, lightDir), 0.0) * shadow + gi);
}