#version 450

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform SkyUBO {
    float time;
    float timeOfDay;
    float dayNightBlend;
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
    vec4 nebulaColorNight;
    vec4 nebulaColorDay;
    vec4 auroraColor;
    float vignetteStrength;
    float overallBrightness;
    mat4 inverseView;   // ← New: from camera
} ubo;

layout(set = 0, binding = 1) uniform samplerCube starCubemap;

// === Noise functions (unchanged) ===
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

// Simple fBM for nebula (better than simple layers)
float fbm(vec3 p, int octaves) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    for (int i = 0; i < octaves; i++) {
        value += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    return value;
}

void main() {
    vec2 uv = vUV * 2.0 - 1.0;

    vec3 rd = normalize(vec3(-uv, 1.6));                    // tweak 1.6 if FOV feels wrong
    vec3 dir = (ubo.inverseView * vec4(rd, 0.0)).xyz;
    dir = normalize(dir);

    float t = ubo.time * ubo.nebulaSpeed;
    vec3 col = vec3(0.0);

    // === Nebula with fBM (much smoother, fewer sharp lines) ===
    vec3 nebulaColor = mix(ubo.nebulaColorNight.rgb, ubo.nebulaColorDay.rgb, ubo.dayNightBlend);

    for (float layer = 0.0; layer < ubo.nebulaLayerCount; layer++) {
        vec3 offset = hash33(vec3(layer * 122.3, layer * 82.7, layer * 17.1)) * 10.0;
        vec3 p = dir * ubo.nebulaScale * (1.8 + layer * 0.6) + offset + t * vec3(0.01, 0.008, 0.015);

        float n = fbm(p, 6);
        n = pow(n * 1.1, 2.6) * 0.085;           // contrast

        col += nebulaColor * n * ubo.nebulaIntensity * (0.65 + 0.35 * ubo.dayNightBlend);
    }

    // === Best round stars version ===
    vec3 stars = vec3(0.0);

    for (float i = 0.0; i < min(ubo.starCount, 1000.0); i++) {
        vec3 h = hash33(vec3(i, i*1.6180339887, i*2.718281828));

        vec3 starPos = normalize(h * 2.0 - 1.0);

        float d = max(dot(dir, starPos), 0.0);
        float brightness = pow(h.z, 30.0) * ubo.starBrightness;

        brightness *= 0.8 + 0.2 * sin(t * ubo.starTwinkleSpeed * (1.0 + h.x*8.0) + i);

        // Very round and sharp star shape
        float star = pow(d, 1.0 / (ubo.starSize * 0.015 + 0.0005));

        stars += brightness * star;
    }

    col += stars * (1.0 - ubo.dayNightBlend * 0.6);

    // === Aurora ===
    float aurora = pow(max(sin(dir.y * 8.0 + t * ubo.auroraSpeed) * 0.5 + 0.5, 0.0), 2.3);
    col += ubo.auroraColor.rgb * aurora * ubo.auroraIntensity * (1.0 - ubo.dayNightBlend);

    // === Final ===
    col *= 1.0 - length(uv) * ubo.vignetteStrength * 0.8;
    col *= ubo.overallBrightness;

    outColor = vec4(col, 1.0);
}