#version 450
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.0, -1.0));
    float diffuse = max(dot(normalize(fragNormal), lightDir), 0.0);

    vec3 base = fragColor.rgb;
    outColor = vec4(base * (0.3 + 0.7 * diffuse), fragColor.a);
}