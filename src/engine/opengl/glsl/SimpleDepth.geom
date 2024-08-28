#version 460 core

layout(points) in;
layout(points, max_vertices = 1) out;

in int instanceID[];

void main() {
    gl_Layer = instanceID[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    EndPrimitive();
}
