#version 450

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    float time;
    float yaw;
    float pitch;
} pc;

vec3 hash33(vec3 p) {
    p = fract(p * vec3(0.1031, 0.1030, 0.0973));
    p += dot(p, p.yxz + 33.33);
    return fract((p.xxy + p.yxx) * p.zyx);
}

float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float n = mix(mix(mix(hash33(i + vec3(0,0,0)).x, hash33(i + vec3(1,0,0)).x, f.x),
                      mix(hash33(i + vec3(0,1,0)).x, hash33(i + vec3(1,1,0)).x, f.x), f.y),
                  mix(mix(hash33(i + vec3(0,0,1)).x, hash33(i + vec3(1,0,1)).x, f.x),
                      mix(hash33(i + vec3(0,1,1)).x, hash33(i + vec3(1,1,1)).x, f.x), f.y), f.z);
    return n;
}

void main() {
    vec2 uv = vUV * 2.0 - 1.0;
    float t = pc.time * 0.03;

    // Apply camera rotation to UV (makes sky feel 3D)
    float cy = cos(pc.pitch);
    float sy = sin(pc.pitch);
    float cx = cos(pc.yaw);
    float sx = sin(pc.yaw);

    vec3 dir = vec3(uv.x * cx - uv.y * sx * sy, 
                    uv.y * cy, 
                    -uv.x * sx - uv.y * cx * sy);

    vec3 col = vec3(0.0);

    // Soft nebula layers
    for (int i = 0; i < 4; i++) {
        float fi = float(i);
        vec3 p = dir * (2.2 + fi * 0.9) + vec3(t * 0.12, t * 0.08, t * 0.15);
        float n = noise(p * 2.8);
        n = pow(n, 2.1) * 0.75;
        vec3 color = mix(vec3(0.6, 0.3, 0.9), vec3(0.2, 0.7, 1.0), fi / 3.0);
        col += color * n * 0.6;
    }

    // Soft stars
    vec3 stars = vec3(0.0);
    for (int i = 0; i < 100; i++) {
        vec3 h = hash33(vec3(i, i*1.6, i*2.8));
        vec3 pos = normalize(h * 2.0 - 1.0);
        float brightness = pow(h.z, 32.0) * (0.7 + 0.3 * sin(t * 6.0 + i));
        float dist = length(dir - pos);
        stars += brightness * (1.0 - smoothstep(0.0, 0.12, dist));
    }
    col += stars * 1.1;

    // Gentle aurora
    float aurora = sin(dir.y * 7.0 + t * 1.1) * 0.5 + 0.5;
    col += vec3(0.35, 0.9, 0.75) * aurora * 0.11;

    // Vignette
    col *= 1.0 - length(uv) * 0.4;

    outColor = vec4(col, 1.0);
}