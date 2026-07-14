#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;   // (ready for texturing later)
layout(location = 2) in vec4 fragColor;
layout(location = 3) in vec3 fragPos;        // ← ADD THIS (world-space position)

layout(location = 0) out vec4 outColor;

struct DirectionalLight {
    vec3 direction;   // direction *towards* the light (matches your original hardcoded lightDir)
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;   // attenuation coefficients (tune per light)
    float linear;
    float quadratic;
};

layout(set = 0, binding = 0) uniform LightingUBO {
    vec3 ambient;                    // incoming ambient light strength × color (e.g. vec3(0.15))
    DirectionalLight dirLight;       // your main directional light
    int numPointLights;              // 0–4 (max for performance)
    PointLight pointLights[4];
    vec3 viewPos;                    // camera position in world space (needed for specular + point lights)
} lighting;

// Optimal classic real-time lighting model (Blinn-Phong)
// Ambient + Directional + Multiple Point lights with proper attenuation
// Specular added for visual quality
void main() {
    vec3 norm    = normalize(fragNormal);
    vec3 viewDir = normalize(lighting.viewPos - fragPos);
    vec3 base    = fragColor.rgb;

    // Start with ambient term (your "incoming ambient light strength")
    vec3 result = base * lighting.ambient;

    // ───── Main Directional Light ─────
    {
        vec3 lightDir = normalize(lighting.dirLight.direction);
        float diff = max(dot(norm, lightDir), 0.0);
        // Diffuse
        vec3 diffuse = diff * base * lighting.dirLight.color * lighting.dirLight.intensity;
        // Blinn-Phong specular (much nicer than Phong, still cheap)
        vec3 halfDir = normalize(lightDir + viewDir);
        float spec   = pow(max(dot(norm, halfDir), 0.0), 32.0);   // shininess = 32 (feel free to expose as uniform)
        vec3 specular = spec * lighting.dirLight.color * lighting.dirLight.intensity * 0.5; // specular strength
        result += diffuse + specular;
    }

    // ───── Point Lights (up to 4) ─────
    for (int i = 0; i < lighting.numPointLights && i < 4; ++i) {
        PointLight p = lighting.pointLights[i];
        vec3  toLight = p.position - fragPos;
        float dist    = length(toLight);
        vec3  lightDir = normalize(toLight);

        // Attenuation (physically plausible inverse-square falloff)
        float attenuation = 1.0 / (p.constant + p.linear * dist + p.quadratic * dist * dist);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * base * p.color * p.intensity * attenuation;

        // Blinn-Phong specular again
        vec3 halfDir = normalize(lightDir + viewDir);
        float spec   = pow(max(dot(norm, halfDir), 0.0), 32.0);
        vec3 specular = spec * p.color * p.intensity * 0.5 * attenuation;
        result += diffuse + specular;
    }

    // Distance fog
    float viewDistance = length(lighting.viewPos - fragPos);

    float fogStart = 10.0;
    float fogEnd   = 1000.0;

    float fogFactor = smoothstep(
        fogStart,
        fogEnd,
        viewDistance
    );

    vec3 fogColor = vec3(0.005, 0.006, 0.008);

    result = mix(
        result,
        fogColor,
        fogFactor
    );

    outColor = vec4(result, fragColor.a);
}