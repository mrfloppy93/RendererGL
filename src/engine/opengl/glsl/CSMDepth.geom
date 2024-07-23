#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 30) out;

layout (std140, binding = 0) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};

uniform int cascadeCount;

void main()
{
    for (int layer = 0; layer < cascadeCount; ++layer)
    {
        for (int i = 0; i < 3; ++i)
        {
            gl_Position =
            lightSpaceMatrices[layer] * gl_in[i].gl_Position;
            gl_Layer = layer;
            EmitVertex();
        }
        EndPrimitive();
    }
}