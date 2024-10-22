#version 450 core

#define MAX_CASCADES 3

layout(triangles) in;
layout(triangle_strip, max_vertices = 3 * MAX_CASCADES) out;

uniform mat4 lightSpaceMatrices[16];

void main() {
    for(int c = 0; c < MAX_CASCADES; ++c) {
        gl_Layer = c;
        for(int i = 0; i < gl_in.length(); ++i) {
            gl_Position = lightSpaceMatrices[c] * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}
