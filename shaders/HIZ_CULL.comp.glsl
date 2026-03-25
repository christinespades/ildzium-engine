#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(local_size_x = 32) in;

layout(set = 0, binding = 0) readonly buffer ObjectData {
    vec3 aabbMin;
    vec3 aabbMax;
    mat4 modelMatrix;
} objects[];

layout(set = 0, binding = 1) uniform sampler2D hizTexture;

struct IndirectCommand {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int vertexOffset;
    uint firstInstance;
    uint materialIndex;
};

layout(set = 0, binding = 2) buffer IndirectCommands {
    IndirectCommand commands[];
} indirect;

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
    vec2 screenSize;
    uint hizMipCount;
    uint objectCount;
} push;

void main() {
    uint objIdx = gl_GlobalInvocationID.x;
    if (objIdx >= push.objectCount) return;
    //if (indirect.commands[objIdx].instanceCount == 1) return;

    vec3 aabbMin = objects[nonuniformEXT(objIdx)].aabbMin;
    vec3 aabbMax = objects[nonuniformEXT(objIdx)].aabbMax;
    mat4 mvp = push.viewProj * objects[nonuniformEXT(objIdx)].modelMatrix;

    vec3 corners[8] = {
        vec3(aabbMin.x, aabbMin.y, aabbMin.z),
        vec3(aabbMax.x, aabbMin.y, aabbMin.z),
        vec3(aabbMin.x, aabbMax.y, aabbMin.z),
        vec3(aabbMax.x, aabbMax.y, aabbMin.z),
        vec3(aabbMin.x, aabbMin.y, aabbMax.z),
        vec3(aabbMax.x, aabbMin.y, aabbMax.z),
        vec3(aabbMin.x, aabbMax.y, aabbMax.z),
        vec3(aabbMax.x, aabbMax.y, aabbMax.z)
    };

    vec2 screenMin = vec2(1.0);
    vec2 screenMax = vec2(0.0);
    float minZ = 1.0;
    bool isVisible = false;

    for (int i = 0; i < 8; ++i) {
        vec4 clip = mvp * vec4(corners[i], 1.0);
        //if (clip.w <= 0.0) continue; // Behind camera
        clip /= clip.w;
        //if (clip.x < -1.0 || clip.x > 1.0 || clip.y < -1.0 || clip.y > 1.0 || clip.z < 0.0 || clip.z > 1.0) continue;
        vec2 ndc = clip.xy * 0.5 + 0.5;
        screenMin = min(screenMin, ndc);
        screenMax = max(screenMax, ndc);
        minZ = min(minZ, clip.z);
        isVisible = true;
    }

    if (isVisible) {
        vec2 boundsSize = (screenMax - screenMin) * push.screenSize;
        float mipLevel = clamp(log2(max(boundsSize.x, boundsSize.y)), 0.0, float(push.hizMipCount - 1));
        vec2 uvs[4] = {
            screenMin,
            vec2(screenMin.x, screenMax.y),
            vec2(screenMax.x, screenMin.y),
            screenMax
            };
        float hizDepth = 1.0;
        for (int i = 0; i < 4; ++i) {
            hizDepth = min(hizDepth, textureLod(hizTexture, uvs[i], mipLevel).r);
        }
        isVisible = (minZ <= hizDepth); // <=
    }

    indirect.commands[objIdx].instanceCount = isVisible ? 1 : 0;
}