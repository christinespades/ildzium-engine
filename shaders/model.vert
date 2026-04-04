#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec4 fragColor;
layout(location = 3) out vec3 fragPos;

struct InstanceData {
    mat4 model;     // will be column-major after transpose on CPU
    vec4 color;
};

// set 0 is lights
layout(set = 1, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} camera;

layout(set = 1, binding = 1, std430) readonly buffer InstanceSSBO {
    InstanceData instances[];
} instanceData;

void main() {
    InstanceData inst = instanceData.instances[gl_InstanceIndex];

    mat4 model = inst.model; // already transposed on CPU

    mat4 mvp = camera.proj * camera.view * model;
    gl_Position = mvp * vec4(inPosition, 1.0);

    // Proper normal transform (handles non-uniform scale)
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    fragNormal = normalMatrix * inNormal;
    fragTexCoord = inTexCoord;
    fragColor = inst.color;
    fragPos = vec3(model * vec4(inPosition, 1.0));
}