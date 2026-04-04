#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec4 fragColor;

struct InstanceData {
    mat4 model;
    vec4 color;
};

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} camera;

layout(set = 0, binding = 1, std140) readonly buffer InstanceSSBO {
    InstanceData instances[];
} instanceData;

void main() {
    InstanceData inst = instanceData.instances[gl_InstanceIndex];

    mat4 mvp = camera.proj * camera.view * inst.model;
    gl_Position = mvp * vec4(inPosition, 1.0);

    fragNormal = inNormal;
    fragTexCoord = inTexCoord;
    fragColor = inst.color;
}