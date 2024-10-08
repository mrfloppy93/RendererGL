#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 lightSpaceMatrix;

void main() {

    for(int i = 0; i < 3; ++i) {
        gl_Position = lightSpaceMatrix * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
