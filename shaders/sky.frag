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

    vec4 nebulaColorNight; 
    vec4 nebulaColorDay;
    vec4 auroraColor;

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
    // 1. Correct Aspect Ratio (optional but prevents stretching)
    // float aspect = 1920.0 / 1080.0; // Use actual resolution or pass via UBO
    vec2 uv = vUV * 2.0 - 1.0;
    // uv.x *= aspect; 

    // 2. Generate initial ray direction (Forward is Z+)
    // Imagine a plane in front of the camera with a 90-degree FOV
    vec3 rd = normalize(vec3(uv, 1.5)); 

    // 3. Rotate ray by Pitch (around X axis)
    float cp = cos(ubo.pitch), sp = sin(ubo.pitch);
    vec3 rayPitch = vec3(
        rd.x,
        rd.y * cp - rd.z * sp,
        rd.y * sp + rd.z * cp
    );

    // 4. Rotate ray by Yaw (around Y axis)
    float cy = cos(ubo.yaw), sy = sin(ubo.yaw);
    vec3 dir = vec3(
        rayPitch.x * cy + rayPitch.z * sy,
        rayPitch.y,
        -rayPitch.x * sy + rayPitch.z * cy
    );

    // Use 'dir' for all your noise and star calculations
    float t = ubo.time;
    vec3 col = vec3(0.0);

    // Nebula - using .rgb since we switched to vec4
    vec3 nebulaColor = mix(ubo.nebulaColorNight.rgb, ubo.nebulaColorDay.rgb, ubo.dayNightBlend);
    for (float i = 0.0; i < ubo.nebulaLayerCount; i++) {
        vec3 p = dir * (2.2 + i * 0.9) + vec3(t * 0.02, t * 0.01, t * 0.03);
        float n = noise(p * ubo.nebulaScale);
        n = pow(n, 2.1) * 0.75;
        col += nebulaColor.rgb * n * ubo.nebulaIntensity * (0.6 + 0.4 * ubo.dayNightBlend);
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
    col += ubo.auroraColor.rgb * aurora * ubo.auroraIntensity * (1.0 - ubo.dayNightBlend);

    // Vignette
    col *= 1.0 - length(uv) * ubo.vignetteStrength;

    col *= ubo.overallBrightness;

    outColor = vec4(col, 1.0);
}