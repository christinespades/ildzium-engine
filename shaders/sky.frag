#version 450

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    float time;
} pc;

vec3 hash33(vec3 p) {
    p = fract(p * vec3(0.1031, 0.1030, 0.0973));
    p += dot(p, p.yxz + 33.33);
    return fract((p.xxy + p.yxx) * p.zyx);
}

void main() {
    vec2 uv = vUV * 2.0 - 1.0;
    float t = pc.time * 0.1;

    // Base nebula layers
    vec3 col = vec3(0.0);
    for (int i = 0; i < 3; i++) {
        float fi = float(i);
        vec2 p = uv * (2.0 + fi * 1.5) + vec2(t * 0.2 + fi, t * 0.1);
        float noise = hash33(vec3(p * 5.0, t * 0.3 + fi)).x;
        noise = pow(noise, 3.0);
        vec3 nebulaColor = mix(vec3(0.6, 0.2, 0.8), vec3(0.1, 0.6, 1.0), fi / 2.0);
        col += nebulaColor * noise * 0.8;
    }

    // Stars
    vec3 stars = vec3(0.0);
    for (int i = 0; i < 80; i++) {
        vec3 h = hash33(vec3(i, i*1.3, i*2.7));
        vec2 pos = h.xy * 2.0 - 1.0;
        float brightness = pow(h.z, 20.0) * (0.7 + 0.3 * sin(t * 10.0 + i));
        float dist = length(uv - pos);
        stars += vec3(1.0) * brightness * (1.0 - smoothstep(0.0, 0.03, dist));
    }
    col += stars * 1.5;

    // Subtle moving aurora / color pulse
    float aurora = sin(uv.y * 8.0 + t * 2.0) * 0.5 + 0.5;
    col += vec3(0.3, 0.8, 0.6) * aurora * 0.15 * (0.5 + 0.5 * sin(t));

    // Vignette
    col *= 1.0 - length(uv) * 0.3;

    outColor = vec4(col, 1.0);
}