#version 450
layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform SkyUBO {
    float time;
    float yaw;
    float pitch;
    float timeOfDay;
    float dayNightBlend;        // 0 = night, 1 = day

    float nebulaSpeed;
    float nebulaScale;
    float nebulaIntensity;
    float nebulaLayerCount;

    float starCount;
    float starBrightness;
    float starTwinkleSpeed;
    float starSize;

    float auroraIntensity;
    float auroraSpeed;

    vec3 nebulaColorNight;
    vec3 nebulaColorDay;
    vec3 auroraColor;

    float vignetteStrength;
    float overallBrightness;
} ubo;
// (hash33 and noise functions stay exactly the same)
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

    float t = ubo.time * 0.03;

    // Camera direction
    float cy = cos(ubo.pitch), sy = sin(ubo.pitch);
    float cx = cos(ubo.yaw),   sx = sin(ubo.yaw);
    vec3 dir = normalize(vec3(uv.x * cx - uv.y * sx * sy,
                              uv.y * cy,
                             -uv.x * sx - uv.y * cx * sy));

    vec3 col = vec3(0.0);

    // Nebula with day/night color lerp
    vec3 nebulaColor = mix(ubo.nebulaColorNight, ubo.nebulaColorDay, ubo.dayNightBlend);
    for (float i = 0.0; i < ubo.nebulaLayerCount; i++) {
        vec3 p = dir * (2.2 + i * 0.9) + vec3(t * 0.12, t * 0.08, t * 0.15);
        float n = noise(p * ubo.nebulaScale);
        n = pow(n, 2.1) * 0.75;
        col += nebulaColor * n * ubo.nebulaIntensity * (0.6 + 0.4 * ubo.dayNightBlend);
    }

    // Stars (stronger at night)
    vec3 stars = vec3(0.0);
    for (float i = 0.0; i < ubo.starCount; i++) {
        vec3 h = hash33(vec3(i, i*1.6, i*2.8));
        vec3 pos = normalize(h * 2.0 - 1.0);
        float brightness = pow(h.z, 32.0) * (0.7 + 0.3 * sin(t * ubo.starTwinkleSpeed + i));
        float dist = length(dir - pos);
        stars += brightness * (1.0 - smoothstep(0.0, ubo.starSize, dist));
    }
    col += stars * ubo.starBrightness * (1.0 - ubo.dayNightBlend * 0.7);  // dim during day

    // Aurora (only at night)
    float aurora = sin(dir.y * 7.0 + t * ubo.auroraSpeed) * 0.5 + 0.5;
    col += ubo.auroraColor * aurora * ubo.auroraIntensity * (1.0 - ubo.dayNightBlend);

    // Vignette
    col *= 1.0 - length(uv) * ubo.vignetteStrength;

    col *= ubo.overallBrightness;

    outColor = vec4(col, 1.0);
}