#version 450

vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0), // Bottom-left (top-left in NDC)
    vec2(-1.0,  3.0), // Top-left (bottom-left in NDC)
    vec2( 3.0, -1.0)  // Bottom-right (top-right in NDC)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}