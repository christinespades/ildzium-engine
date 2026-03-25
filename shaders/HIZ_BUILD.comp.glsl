#version 450
#extension GL_EXT_nonuniform_qualifier : require
layout(local_size_x = 8, local_size_y = 8) in;

layout(set = 0, binding = 1) uniform sampler2D hizTexture[]; // Array of mip levels
layout(set = 0, binding = 3) uniform sampler2D depthTexture;
layout(set = 0, binding = 4) writeonly uniform image2D hizTextureImage[];

layout(push_constant) uniform PushConstants {
    vec2 screenSize;
    uint mipLevel;
} push;

void main() {
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 texSize = ivec2(push.screenSize) >> push.mipLevel;
    if (coord.x >= texSize.x || coord.y >= texSize.y) return;

    vec2 uv = (vec2(coord) + 0.5) / vec2(texSize);
    vec4 depths;

    if (push.mipLevel == 0) {
        depths = textureGather(depthTexture, uv, 0);
    } else {
        depths = textureGather(hizTexture[push.mipLevel - 1], uv, 0);
    }

    float maxDepth = max(max(depths.x, depths.y), max(depths.z, depths.w));
    imageStore(hizTextureImage[push.mipLevel], coord, vec4(maxDepth));
}