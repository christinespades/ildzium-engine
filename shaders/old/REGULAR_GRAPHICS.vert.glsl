#version 450
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in uint inMatIdx;

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
    vec3 camPos;
    uint vizMode;
    float curT;
} pc;

layout(binding = 1) buffer Xforms {
    mat4 xforms[];
} xforms;

layout(location = 0) out vec3 outNormal;
layout(location = 1) flat out uint outMatIdx;
layout(location = 2) out vec3 outBarycentric;
layout(location = 3) out vec2 fragUV;
layout(location = 4) out vec3 outWorldPos;

void main() {
    outMatIdx = inMatIdx;
    uint meshIndex = gl_InstanceIndex;
    mat4 xform = xforms.xforms[meshIndex];
    vec4 worldPos = xform * vec4(inPos, 1.0);
    gl_Position = pc.viewProj * worldPos;
    outNormal = normalize(mat3(xform) * inNormal);
    fragUV = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;
    outWorldPos = worldPos.xyz;

    if (pc.vizMode == 2) { // Vertex proximity
        if (gl_VertexIndex % 3 == 0) {
            outBarycentric = vec3(1.0, 0.0, 0.0); // Vertex 1
        } else if (gl_VertexIndex % 3 == 1) {
            outBarycentric = vec3(0.0, 1.0, 0.0); // Vertex 2
        } else {
            outBarycentric = vec3(0.0, 0.0, 1.0); // Vertex 3
        }
    } else if (pc.vizMode == 4) { // Bounds
        outBarycentric = vec3(0.0, 0.0, 0.0);
    } else {
        outBarycentric = vec3(0.0, 0.0, 0.0);
    }
}