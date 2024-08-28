#version 460 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 lightSpaceMatrices[16];

out int instanceID;

void main()
{
    instanceID = gl_InstanceID;
    gl_Position = lightSpaceMatrices[gl_InstanceID] * model * vec4(aPos, 1.0);
}  