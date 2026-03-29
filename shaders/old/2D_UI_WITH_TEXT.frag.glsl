#version 450
layout(push_constant) uniform PushConstants {
    float windowWidth;
    float windowHeight;
    float curT;
    uint ctxt;
    float uiScale;
} pc;

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in float inStyle;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D fontTexture;

float dither(vec2 position) {
    return fract(sin(dot(position, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    float dayCycle = 1.0 - abs(2.0 * fract(pc.curT / 24.0 * 2) - 1.0);
    float dayCycle2 = 1.0 - abs(2.0 * fract(pc.curT / 24.0 * 3) - 1.0);
    float dayCycle3 = 1.0 - abs(2.0 * fract(pc.curT / 24.0) - 1.0);

    float boxAlpha = 0.9;

    float scrollGradient = mix(0.85, 1.0, inUV.y);
    float reflection = smoothstep(0.15, 0.5, inUV.y) * smoothstep(0.95, 0.5, inUV.y);
    vec3 scrollColor = 0.5 * scrollGradient + vec3(0.5) * reflection;
    float scrollEdgeFade = smoothstep(0.0, 0.1, inUV.x) * smoothstep(1.0, 0.5, inUV.x);
    float scrollAlpha = 0.5 * mix(0.1, 0.9, scrollEdgeFade);

    if (inStyle < 1.0) { // Border
        float gradient = mix(0.0, -0.3, inUV.x);
        float bevel = smoothstep(0.0, 0.45, inUV.x) + smoothstep(0.0, 0.95, inUV.y);
        float innerGlow = 0.05 * sin(inUV.y * -99.0) * dayCycle * 12;
        vec3 baseColor = vec3(0.42, 0.42, 0.42) * gradient + vec3(0.53) * bevel * -0.83 + innerGlow * -0.4;
        float edgeFade = smoothstep(0.0, 2.5, inUV.x) * smoothstep(0.0, 0.45, inUV.x);
        float alpha = 2.1 * mix(0.015, -2.0, edgeFade);
        outColor = vec4(baseColor, alpha);

        float gradient2 = mix(0.4, 0.09, inUV.y);

        float noise = fract(sin(dot(inUV * 350.0, vec2(12.9898, 78.233))) * 43758.5453);
        float fineNoise = fract(sin(dot(inUV * 300.0, vec2(4.398, 7.23))) * 31278.5453);
        
        float veins = abs(sin((inUV.x * 2220.0) + sin(inUV.y * 220.0)));
        veins = smoothstep(0.03, 1.96, veins);

        float pattern = noise * 0.62 + fineNoise * 0.1 + veins * 1.97;

        vec3 baseColor2 = vec3(0.12, 0.07, 0.095) * gradient2 + vec3(pattern * 0.43);
        outColor = mix(vec4(outColor), vec4(baseColor2, 0.3), 0.58);
        if (inStyle > 2.0) { // Slider handle
            outColor *= vec4(1.0, 0.93, 0.79, 12.3);
        }

    } else if (inStyle < 1.0) { // Scrollbar handle
        outColor = vec4(scrollColor, scrollAlpha * 2);
        outColor = vec4(1.0);

    } else if (inStyle < 8.6) { // Scrollbar track
        if (pc.ctxt == 0) { // No scrollbar on main menu
            outColor = vec4(0.0);
        } else {
        outColor = vec4(scrollColor, scrollAlpha);
        }

    } else if (inUV == vec2(0.0, 0.0)) { // Draw box
        vec2 screenUV = gl_FragCoord.xy / vec2(-1.01);
        vec2 boxUV = fract(screenUV * (pc.curT / 1000000000 * 910));
        float noise = fract(sin(dot(boxUV, vec2(0.1))));
        vec2 burnCenter1 = vec2(0.3, 0.5);
        vec2 burnCenter2 = vec2(0.7, 0.2);
        vec2 burnCenter3 = vec2(0.5, 0.8);
        float burn1 = smoothstep(0.2, 0.05, length(boxUV - burnCenter1));
        float burn2 = smoothstep(0.25, 0.07, length(boxUV - burnCenter2));
        float burn3 = smoothstep(0.18, 0.04, length(boxUV - burnCenter3));
        float burnMask = burn1 + burn2 + burn3;
        float cracks = sin(20.0 * boxUV.x + sin(boxUV.y * 40.0)) * cos(60.0 * boxUV.y);
        cracks = smoothstep(0.05, 0.01, abs(cracks));
        float detail = noise * dayCycle2;
        detail -= burnMask;     // Strong intensity in burn areas
        detail -= cracks;       // Visible but subtle cracks
        detail += burnMask; // Flickering glow in burned areas
        detail = mix(detail, noise, 0.69);

        vec4 ctxtColor;
        if (pc.ctxt == 0) { // Main menu
            ctxtColor = vec4(0.15, 0.24, 0.14, boxAlpha);
        } else if (pc.ctxt == 1) { // Debug
            ctxtColor = vec4(0.24, 0.14, 0.15, boxAlpha);
        } else if (pc.ctxt == 2) { // Settings
            ctxtColor = vec4(0.14, 0.15, 0.24, boxAlpha);
        } else if (pc.ctxt == 3) { // No UI
            ctxtColor = vec4(0.0, 0.0, 0.0, 1.0);
        } else if (pc.ctxt == 4) { // No UI
            ctxtColor = vec4(0.0, 0.0, 0.0, 1.0);
        } else if (pc.ctxt == 5) { // No UI
            ctxtColor = vec4(0.0, 0.0, 0.0, 1.0);
        } else if (pc.ctxt == 6) { // No UI
            ctxtColor = vec4(0.0, 0.0, 0.0, 1.0);
        } else if (pc.ctxt == 7) { // No UI
            ctxtColor = vec4(0.0, 0.0, 0.0, 1.0);
        }
        if (inStyle < 1.0) { // slider handle
            outColor = detail * vec4(0.99, 0.97, 0.95, 12.99);
        } else if (inStyle < 1.0) { // dropdown menu
            outColor = vec4(0.195, 0.18, 0.175, 0.2);
        } else if (inStyle < 1.0) { // dropdown elements
            outColor = vec4(0.235, 0.22, 0.215, 0.2);
        } else {
            outColor = vec4(mix(ctxtColor.rgb, vec3(0.1, 1.0, 1.0), ctxtColor.a + 0.95) * (dayCycle / 224 + 0.9) + vec3(detail * 20.97), -detail + 0.05);
            outColor.rgb -= 0.3;
            outColor = mix(mix(vec4(0.0, 0.0, 1.0, 1.0), clamp(outColor, 0.01, 0.92), 0.5), vec4(clamp(outColor.rgb, 0.0, 0.01), 1.0), 0.82);
            outColor.a *= 0.1;
        }

    } else { // Text
        float sdf = texture(fontTexture, inUV).x;
        float scale = (pc.uiScale * 0.1) + 0.9;
        float edge = 0.5 / scale;
        float smoothing = 0.06 / scale;
        float alpha = smoothstep(edge - smoothing, edge + smoothing, sdf);
        if (alpha < 0.1) discard;
        outColor = vec4(1.0, 1.0, 1.0, alpha);
        return;
    }
    outColor += dither(gl_FragCoord.xz * 120.1) * 0.001;
    outColor += dither(gl_FragCoord.xy * 20.1) * 0.001;
}
