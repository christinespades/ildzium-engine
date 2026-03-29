#version 450
layout(location = 0) out vec2 fragUV;

vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0), // bottom left
    vec2(-1.0,  3.0), // top left outside clip space (reversed)
    vec2( 3.0, -1.0)  // bottom right outside clip space
);

vec2 uvs[3] = vec2[](
    vec2(0.0, 0.0),   // bottom left
    vec2(0.0, 2.0),   // top left (reversed)
    vec2(2.0, 0.0)    // bottom right
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragUV = uvs[gl_VertexIndex];
}
