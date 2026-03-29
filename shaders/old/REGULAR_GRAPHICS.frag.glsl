#version 450
layout(location = 0) in vec3 inNormal;
layout(location = 1) flat in uint inMaterialIndex;
layout(location = 2) in vec3 inBarycentric;
layout(location = 3) in vec2 fragUV;
layout(location = 4) in vec3 inWorldPos;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
    vec3 camPos;
    uint vizMode;
    float curT;
} pc;

layout(binding = 0) buffer Mats { vec4 cols[]; } mats;

float curve(float x, float strength) {
    return mix(x, x * x, strength);
}

vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 29.0)) * 289.0; }
vec4 mod289(vec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 permute(vec4 x) { return mod289(((x * 4.0) + 1.0) * x); }
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }

float snoise(vec3 v) {
    const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0);
    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

    vec3 i = floor(v + dot(v, C.yyy));
    vec3 x0 = v - i + dot(i, C.xxx);

    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min(g.xyz, l.zxy);
    vec3 i2 = max(g.xyz, l.zxy);

    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy;
    vec3 x3 = x0 - D.yyy;

    i = mod289(i);
    vec4 p = permute(permute(permute(
                i.z + vec4(0.0, i1.z, i2.z, 1.0))
                + i.y + vec4(0.0, i1.y, i2.y, 1.0))
                + i.x + vec4(0.0, i1.x, i2.x, 1.0));

    float n_ = 0.142857142857;
    vec3 ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_);

    vec4 x = x_ * ns.x + ns.yyyy;
    vec4 y = y_ * ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);

    vec4 s0 = floor(b0) * 2.0 + 1.0;
    vec4 s1 = floor(b1) * 2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    vec3 p0 = vec3(a0.xy, h.x);
    vec3 p1 = vec3(a0.zw, h.y);
    vec3 p2 = vec3(a1.xy, h.z);
    vec3 p3 = vec3(a1.zw, h.w);

    vec4 norm = taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    vec4 m = max(0.67 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m * m, vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}

float noise(vec3 p, float scale) {
    return snoise(p * scale) * 0.5 + 0.5;
}

float crackPattern(vec3 pos, float scale, float thickness) {
    vec3 warped = pos + vec3(sin(pos.y), cos(pos.x * 0.01), sin(pos.z));
    vec3 grid = abs(fract(warped * scale - 0.5) - 0.5) / fwidth(warped * scale);
    float line = min(min(grid.x, grid.y), grid.z);
    return smoothstep(thickness, 0.0, line);
}

float hash2(vec2 p) {
    return fract(sin(dot(p ,vec2(127.1, 311.7))) * 43758.5453);
}
float dither(vec2 position) {
    return fract(sin(dot(position, vec2(12.9898, 78.233))) * 43758.5453);
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

const float PI = 3.1415926535;

void main() {
    vec3 N = normalize(inNormal);
    float t = pc.curT * 0.3;
    float hour = mod(t, 23.9);
    float sunAngle = (hour / 23.9) * 2.0 * PI;
    vec3 sunDir = vec3(0.0, -sin(sunAngle), cos(sunAngle));
    vec3 L = sunDir;
    vec3 V = normalize(L - inWorldPos);
    vec3 H = normalize(L + V);
    float dotNV = max(dot(N, V), 0.0);
    float ambientStrength = 11.0;
    float diffuseStrength = 121.0;
    float specularStrength = 131.0;
    float shininess = 122.0;
    float lightIntensity = 28.8;
    float contrast = 11.0;
    float scale = 21.0;
    float df = length(fwidth(inWorldPos * scale));
    float lod = clamp(1.0 / df, 0.0, 1.0);  // 0 = far (blur), 1 = close (detail)
    float diff = max(dot(N, L), 0.0);
    float curvedDiff = curve(diff, contrast);
    float spec = pow(max(dot(N, H), 0.0), shininess);
    vec4 baseColor = mats.cols[inMaterialIndex] * 0.3;
    vec3 litColor = baseColor.rgb * (ambientStrength + lightIntensity * diffuseStrength * curvedDiff);
    litColor += specularStrength * spec * lightIntensity;
    float grain = noise(vec3(inWorldPos.xyz), 0.25);
    grain *= lod;
    float cracks = crackPattern(inWorldPos, 120.13, 20.1);
    cracks *= lod;
    float crackDarkening = mix(33.01, 1.1 + cracks * 0.008, cracks);
    crackDarkening *= lod;
    float detail = grain * 20.2 * crackDarkening * 20;
    vec3 texturedColor = baseColor.rgb * mix(0.2, 1.2, detail);
    float cracks2 = crackPattern(inWorldPos, 0.93, 2.1);
    texturedColor = texturedColor + cracks2;
    float lightFactor = max(dot(normalize(gl_FragCoord.xyz - L), L), 0.9);
    lightFactor += pow(lightFactor, 1.0) * 0.92;
    vec3 litTexturedColor = litColor * texturedColor;
    float rim = (1.0 - dotNV) * (1.0 - dotNV);
    rim *= lod;
    float sparkle = pow(max(dot(H, N), 0.5), 0.005);
    sparkle *= 0.2 + 0.1 * sin(dot(inWorldPos, vec3(L)));

    vec3 finalColor = litTexturedColor * cheapNoise(vec2(0.0013 + rim)) + vec3(sparkle);

    if (pc.vizMode == 1) { // Normals
        outColor = vec4(N * 0.5 + 0.5, 1.0); // R = X, G = Y, B = Z
    } else if (pc.vizMode == 2) { // Vertices
        float vertexProximity = max(max(inBarycentric.x, inBarycentric.y), inBarycentric.z);
        float vertexThreshold = 0.95; // Adjust this (0.8–1.0) for "point" size
        vec4 debugVertexColor = vec4(1.0, 0.4, 0.6, 1.0); // Color for debug visualization of vertices
        if (vertexProximity > vertexThreshold) {
            float blendFactor = smoothstep(vertexThreshold, 1.0, vertexProximity);
            outColor = mix(vec4(0.1, 0.4, 0.9, 1.0), debugVertexColor, blendFactor);
        } else {
            outColor = mix(vec4(finalColor, 1.0), vec4(0.3, 0.3, 0.3, 1.0), 0.999);
        }
    } else if (pc.vizMode == 3) { //asdfasdasd
        outColor = vec4(0.0, 0.0, 0.0, 1.0);

    } else if (pc.vizMode == 4) { // Bounds
        vec3 someColor = vec3(0.05, 0.05, 0.05);
        for (int i = 0; i < 93; ++i) {
            someColor.rgb += sin(fragUV.x * fragUV.y * float(i));
            someColor.b += sin(float(i));
        }
        outColor.rgb = someColor.rgb;
        outColor.a = 1.0;
    } else {
        outColor = vec4(finalColor, baseColor.a);
    }
    outColor += dither(gl_FragCoord.xy) * 0.003;
}