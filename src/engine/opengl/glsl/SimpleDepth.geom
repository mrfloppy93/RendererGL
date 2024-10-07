#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in int instanceID[];

void main() {
    gl_Layer = instanceID[0];
    for(int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
