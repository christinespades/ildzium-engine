#version 450
layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D renderTexture;

layout(push_constant) uniform PushConstants {
    float brightness;
    float curT;
} pc;

void main() {
    float t = pc.curT;
    vec2 texelSize = 0.35 / textureSize(renderTexture, 0);
    vec3 color = texture(renderTexture, fragUV).rgb;
    outColor = vec4(color, 1.0) * pc.brightness;
}