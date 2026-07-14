#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 viewPos;

layout(location = 0) out vec4 outColor;

float capsuleDistance(vec2 p)
{
    if(p.x < 0.0)
    {
        return length(vec2(p.x, p.y));
    }
    if(p.x > 1.0)
    {
        return length(vec2(p.x - 1.0, p.y));
    }
    return abs(p.y);
}

void main()
{
    float borderThreshold = 0.995;              
    vec3  outlineColor    = vec3(0.0, 0.0, 0.0); 
    float minAllowedAlpha = 0.55;               
    float alphaMultiplier = 0.9;
    float discardCutoff   = 0.001;              
    float fogStartPoint   = 10.0;
    float fogEndPoint     = 10000.0;
    vec3  fogColor        = vec3(0.002, 0.0025, 0.003);

    float distance = capsuleDistance(fragUV);
    float aa = fwidth(distance);
    float alpha = 1.0 - smoothstep(1.0 - aa, 1.0 + aa, distance);
    if(alpha <= discardCutoff)
        discard;

    float borderFactor = smoothstep(borderThreshold - aa, borderThreshold + aa, distance);
    vec3 baseColor = mix(fragColor, outlineColor, borderFactor);
    float viewDistance = length(viewPos - fragPos);
    float fogFactor = smoothstep(fogStartPoint, fogEndPoint, viewDistance);
    vec3 colorWithFog = mix(baseColor, fogColor, fogFactor);
    float distanceFade = 1.0 - fogFactor;
    float finalAlpha = max(alpha * distanceFade * alphaMultiplier, minAllowedAlpha);
    outColor = vec4(colorWithFog, finalAlpha);
}