#version 450

layout(set = 1, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    mat4 inverseView;
    vec2 viewport;
} camera;

layout(location = 0) in vec2 inQuad;

layout(location = 1) in vec3 inStart;
layout(location = 2) in vec3 inEnd;
layout(location = 3) in float inWidth;
layout(location = 4) in vec3 inColor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out vec3 viewPos;

void main()
{
    vec3 lineDir = normalize(inEnd - inStart);
    vec3 startView =
        (camera.view * vec4(inStart,1.0)).xyz;
    vec3 endView =
        (camera.view * vec4(inEnd,1.0)).xyz;
    vec3 viewDir =
        normalize(endView - startView);

    // Camera forward in world space
    vec3 cameraForward =
        -normalize(camera.inverseView[2].xyz);

    vec3 sideWorld =
        normalize(cross(lineDir, cameraForward));

    vec3 offset =
        sideWorld * inWidth * 0.5;

    vec3 position;

    if(inQuad.x < 0.0)
        position = inStart;
    else
        position = inEnd;

    if(inQuad.y < 0.0)
        position -= offset;
    else
        position += offset;

    gl_Position =
        camera.proj *
        camera.view *
        vec4(position,1.0);

    float along =
        (inQuad.x < 0.0) ? 0.0 : 1.0;

    fragUV = vec2(
        along,
        inQuad.y
    );

    fragColor = inColor;
    fragPos = position;
    viewPos = camera.inverseView[3].xyz;
}