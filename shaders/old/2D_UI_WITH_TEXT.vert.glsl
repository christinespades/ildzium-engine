#version 450
layout(push_constant) uniform PushConstants {
    float windowWidth;
    float windowHeight;
    float curT;
    uint ctxt;
    float uiScale;
} pc;

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in float inStyle;

layout(location = 0) out vec2 outPos;
layout(location = 1) out vec2 outUV;
layout(location = 2) out float outStyle;

void main() {
    vec2 scaledPos = inPos * pc.uiScale;
    vec2 ndcPos = (scaledPos / vec2(pc.windowWidth, pc.windowHeight)) * 2.0 - 1.0;
    gl_Position = vec4(ndcPos, 0.0, 1.0);
    outPos = gl_Position.rg;
    outUV = inUV;
    outStyle = inStyle;
}