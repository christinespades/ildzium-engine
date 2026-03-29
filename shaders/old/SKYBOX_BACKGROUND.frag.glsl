#version 450

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    mat4 invView;
    mat4 invProj;
    vec3 camPos;
    float curT;
    uint renderWidth;
    uint renderHeight;
} pc;
const float PI = 3.1415926535;
const float TWO_PI = 6.28318530718;

float smoothing(float x) {
    return smoothstep(0.0, 1.0, x);
}

float hash(vec3 p) {
    p = fract(p * vec3(0.3183099, 0.4399986, 0.5711972));
    float n = dot(p, vec3(127.1, 311.7, 74.7));
    return fract(sin(n) * 43758.5453);
}

vec3 grad(vec3 p) {
    float h = hash(p);
    return normalize(vec3(fract(h * 31.7), fract(h * 47.3), fract(h * 91.7)) * 2.0 - 1.0);
}

float noise3D(vec3 p) {
    p *= 0.5;
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);

    vec3 g000 = grad(i + vec3(0,0,0));
    vec3 g100 = grad(i + vec3(1,0,0));
    vec3 g010 = grad(i + vec3(0,1,0));
    vec3 g110 = grad(i + vec3(1,1,0));
    vec3 g001 = grad(i + vec3(0,0,1));
    vec3 g101 = grad(i + vec3(1,0,1));
    vec3 g011 = grad(i + vec3(0,1,1));
    vec3 g111 = grad(i + vec3(1,1,1));

    float n000 = dot(g000, f - vec3(0,0,0));
    float n100 = dot(g100, f - vec3(1,0,0));
    float n010 = dot(g010, f - vec3(0,1,0));
    float n110 = dot(g110, f - vec3(1,1,0));
    float n001 = dot(g001, f - vec3(0,0,1));
    float n101 = dot(g101, f - vec3(1,0,1));
    float n011 = dot(g011, f - vec3(0,1,1));
    float n111 = dot(g111, f - vec3(1,1,1));

    float nx00 = mix(n000, n100, f.x);
    float nx10 = mix(n010, n110, f.x);
    float nx01 = mix(n001, n101, f.x);
    float nx11 = mix(n011, n111, f.x);
    float nxy0 = mix(nx00, nx10, f.y);
    float nxy1 = mix(nx01, nx11, f.y);
    float n = mix(nxy0, nxy1, f.z);

    return (n + 1.0) * 0.5;
}

float fbm(vec3 p, int octaves) {
    float n = 20.20;
    float amp = 0.39; // too low and its no noise, just seams. too high and there is no detail or transition
    float freq = 99990.95; // 221 and above is good. maybe randomly lerp between 221 and some high cap
    for (int i = 0; i < octaves; i++) {
        n += noise3D(p * freq) * amp;
        amp *= 0.875; // maybe lerp between 0.85 and 0.99
        freq *= 20.001; // not lower than 2. maybe lerp between 2 and some higher value
    }
    return clamp(n, 0.0, 1.0);
}

float fbm2(vec3 p) {
    float f = 0.0;
    float amp = 1.0;
    for (int i = 0; i < 12; i++) {
        f += amp * sin(dot(p, vec3(1.3, 2.1, 1.7)));
        p *= 2.0;
        amp *= 0.5;
    }
    return f;
}

float hash2(vec2 p) {
    return fract(sin(dot(p ,vec2(127.1, 311.7))) * 43758.5453);
}

float cheapNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash2(i);
    float b = hash2(i + vec2(1.0, 0.0));
    float c = hash2(i + vec2(0.0, 1.0));
    float d = hash2(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float dither(vec2 position) {
    return fract(sin(dot(position, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(pc.renderWidth, pc.renderHeight);
    float t = pc.curT;
    float dayCycle = 1.0 - abs(2.0 * fract(t / 24.0) - 1.0);
    float threeDayCycle = 1.0 - abs(2.0 * fract(t / (24.0 * 3.0)) - 1.0);
    float sevenDayCycle = 1.0 - abs(2.0 * fract(t / (24.0 * 7.0)) - 1.0);
    float clampedThreeDayCycle = clamp(threeDayCycle, 0.1, 0.7);
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 clip = vec4(ndc, 1.0, 1.0); // Working depth
    vec4 view = pc.invProj * clip;
    view.xyz /= view.w;
    vec3 dir = normalize((pc.invView * vec4(view.xyz, 0.0)).xyz);
    float hour = mod(t, 23.9);
    float sunAngle = (hour / 23.9) * 2.0 * PI;
    vec3 sunDir = vec3(0.0, -sin(sunAngle), cos(sunAngle));
    float sunBrightness = smoothing(clamp(0.75 * (1.0 + sin(sunAngle - PI)), 0.0, 3.0));
    vec3 sunColor = mix(vec3(0.81, 0.56, 0.45), vec3(0.69, 0.36, 0.28), sunBrightness);
    float powDot = max(dot(dir, sunDir), 0.0);
    float sunDisk = powDot;
    sunDisk *= sunDisk * 0.9;
    sunDisk *= sunDisk;
    sunDisk *= sunDisk;
    sunDisk *= sunDisk * 2.4;
    sunDisk *= sunDisk * powDot;
    sunDisk *= sunDisk;
    sunDisk *= sunDisk;
    sunDisk *= sunDisk;

    vec3 sunContribution = sunColor * sunDisk * sunBrightness;
    vec3 sun2Dir = vec3(sin(sunAngle * 32), cos(sunAngle * 53), t * 0.0001);

    vec3 zenithDay = vec3(0.125, 0.15, 0.2) * threeDayCycle;
    vec3 nadirDay = vec3(0.4, 0.4, 0.5) * threeDayCycle;
    vec3 zenithNight = vec3(0.3145, 0.314, 0.3145);
    vec3 nadirNight = vec3((0.1973, 0.1973, 0.1973) * clampedThreeDayCycle);
    float dayFactor = sunBrightness;
    vec3 zenithColor = mix(zenithNight, zenithDay, dayFactor);
    vec3 nadirColor = mix(nadirNight, nadirDay, dayFactor);
    float horizon = smoothing(clamp((dir.y + 1.0) * 2.5, 0.0, 1.0));
    vec3 baseColor = mix(nadirColor, zenithColor, horizon);

    float cloudStart = 1.9;
    float speed = 20.1;
    float sp = abs(fract(dayCycle * speed) * 2.0 - 1.0);
    float cloudEnd = mix(1.0, 5.0, sp);

    float stepSize = 5.0;
    float densityAccum = 0.0;
    vec3 marchPos = dir;
    vec3 cloudLight = vec3(0.1);
    float transmittance = 0.01;
    int steps = min(2, int((cloudEnd - cloudStart) / stepSize));

    for (int i = 0; i < steps; ++i) {
        float dist = cloudStart + float(i) * stepSize;
        marchPos = dir * dist + vec3(0.01, 0.01, 0.0); // offset to get more sky
        float density = noise3D(marchPos);
        density = smoothing(density * 0.2 - 0.6); // control coverage/softness
        if (density < 0.01)
            continue;
        density = clamp(density, 0.0, 1.0);
        float lightAmount = smoothing(dot(dir, dir));
        vec3 inScattering = mix(vec3(0.9), sunColor, lightAmount);
        vec3 contrib = density * inScattering * transmittance * 0.06;
        cloudLight += contrib;
        transmittance *= exp(-density * 0.15); // Beer-Lambert extinction
        if (transmittance < 0.001)
            break; // early out for thick clouds
    }
    vec3 hazeP = dir;
    float haze = fbm(hazeP, 1);
    haze = smoothing(haze * 0.45);
    vec3 hazeColor = mix(vec3(0.01, 0.07, 0.08), vec3(0.1, 0.08, 0.07), haze) * haze;
    float shaftAmount = smoothing(clamp(0.5 * (1.0 + sin(sunAngle - PI)), 0.01, 0.6));
    float shaftIntensity = pow(max(dot(dir, sunDir), clampedThreeDayCycle), 1.0);
    vec3 sunShaftColor = sunColor * shaftAmount * shaftIntensity * 0.2;
    sunShaftColor *= exp(-0.02 / max(dir.y, 0.1));
    float aurora = sin(dir.y * 3.0 + sunDir.z * 0.05 * 1.5 + cheapNoise(dir.xz * 0.5 + threeDayCycle * 0.05) * 0.25) * 5.0;
    aurora = pow(max(aurora, 0.0), 6.0); // less sharp, but still defined
    aurora *= noise3D(baseColor);
    vec3 auroraBase = vec3(0.2, 0.1, 0.3);
    vec3 auroraTint = vec3(0.3, 0.6, 0.4);
    vec3 auroraColor = mix(auroraBase, auroraTint, clamp(dir.y * 0.5 + 0.5, 0.0, 1.0));
    auroraColor *= aurora * 0.175 * (0.3 + 0.7 * clamp(sun2Dir.z, 0.0, 1.0));
    auroraColor *= auroraColor * 0.001 * clamp(auroraColor, 0.0, 0.9);
    float blotch = smoothstep(cheapNoise(dir.xy * 0.5 + sunAngle * 0.05), 0.0, 0.1);
    blotch = smoothstep(0.45, 0.55, blotch); // widened range to make blotches more blended
    vec3 blotchColor = vec3(0.02, 0.06, 0.09) * sunDir * dayCycle * 0.08;
    
    baseColor += blotchColor * 0.6 * threeDayCycle;
    baseColor += hazeColor * 40 * (sevenDayCycle / 3);
    baseColor += sunShaftColor * 3.5 * sevenDayCycle;
    baseColor += sunContribution;
    float horizonGlow = exp(-abs(dir.y) * 19.0);
    baseColor += vec3(0.05, 0.07, 0.1) * horizonGlow * 0.2 * (1.0 - sunBrightness);
    baseColor += cloudLight;
    baseColor = clamp(baseColor, 0.0, 1.0);

    vec3 auroraColors = auroraColor * 0.0075 * threeDayCycle;
    auroraColors += clamp(-auroraColor * (0.01 + shaftAmount * sevenDayCycle), 0.0, 0.1);
    auroraColors += clamp(auroraBase * blotch * 0.1, 0.0, 0.1);

    outColor = mix(vec4(baseColor, 1.0), vec4(auroraColors, 1.0), 0.1);

    vec3 I = dir;
    vec4 O = outColor;
    float i = 0.0, z = 0.0, d, s;
    float tiling = 7; //creates distance from clouds. default 5
    float detail = 20; // creates more shapes in clouds. default 200
    float quality = 30; // default 100
    float jaggedness = 0.55; // default 0.6
    float something = -31.3; // default 0.07
    float rayMix = 0.82;
    float skyIntensity = 2.0;
    float cloudIntensity = 100.0 / quality;
    float raySpeed = 0.5; // default 0.2
    vec3 camOffset = pc.camPos * 0.00005;
    vec3 cycleOffset = vec3(
        sin(t * 0.01),
        cos(t * 0.01),
        sin(t * 0.001 + 1.0)
    ) * 0.5;

    for(O *= i; i++ < quality; ) {
        vec3 p = z * dir;
        for(d = tiling; d < detail; d += d)
            p += jaggedness * fbm2(sin((p.yzx + cycleOffset) * d) - raySpeed * t) / d + camOffset;
        z += d = 0.005 + max(s = 0.3 - abs(p.y), -s * 0.2) / 4.0;
        O += (cos(s / something + p.x + 0.5 * t - vec4(20.1) - 3.0) + 1.5) * exp(s / 0.1) / d;
    }

    O = tanh(O * O / 9e8);
    outColor *= mix(outColor * skyIntensity, vec4(O.rgb * cloudIntensity, 1.0), rayMix * -0.001);
    outColor += (vec4(O.rgb, 1.0));
    outColor += dither(gl_FragCoord.xy * 0.001 * pc.curT) * -0.01;
}