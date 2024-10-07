#version 450 core

layout(triangles, invocations = 3) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 lightSpaceMatrices[16];

void main() {
    gl_Layer = gl_InvocationID;
    for(int i = 0; i < 3; ++i) {
        gl_Position = lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
