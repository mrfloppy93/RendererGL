#version 330 core

#define MAX_LIGHTS 64

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    //vec4 FragPosLightSpace;
} fs_in;

struct Light {
    vec3 position;
    vec3 color;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    bool pointLight;
};

struct MaterialMaps {
    sampler2D diffuseMap;
    sampler2D specularMap;
    sampler2D normalMap;
    sampler2D depthMap;
    sampler2D emissionMap;
};
uniform MaterialMaps materialMaps;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;
uniform sampler2D shadowMap2;
uniform bool shadowMapping;
uniform mat4 lightSpaceMatrices[2];
uniform float cascadePlaneDistances[1];
uniform int cascadeCount;

uniform vec3 lightPos;

uniform Light lights[MAX_LIGHTS];
uniform int nLights;

uniform vec3 viewPos;
uniform mat4 view;
uniform float farPlane;

float ShadowCalculation(vec3 fragPosWorldSpace, vec3 normal, vec3 lightDir)
{

    vec4 fragPosLightSpace = lightSpaceMatrices[1] * vec4(fragPosWorldSpace, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap2, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = max(0.003 * (1.0 - dot(normal, lightDir)), 0.0005);
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    //float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

/*float ShadowCalculation(vec3 fragPosWorldSpace, vec3 normal, vec3 lightDir)
{
    // select cascade layer
    vec4 fragPosViewSpace = view * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (depthValue < cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = cascadeCount;
    }

    int layer = 0;
    if(depthValue >= cascadePlaneDistances[0]) {
        layer = 1;
    }

    vec4 fragPosLightSpace = lightSpaceMatrix[layer] * vec4(fragPosWorldSpace, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    if(layer != 0) {
     closestDepth = texture(shadowMap2, projCoords.xy).r;
    }
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    if (currentDepth > 1.0)
    {
        return 0.0;
    }

    // check whether current frag pos is in shadow
    float bias = max(0.003 * (1.0 - dot(normal, lightDir)), 0.0005);
    // calculate bias (based on depth map resolution and slope)
    const float biasModifier = 0.5f;
    if (layer == cascadeCount)
    {
        bias *= 1 / (farPlane * biasModifier);
    }
    else
    {
        bias *= 1 / (cascadePlaneDistances[layer] * biasModifier);
    }

    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    //float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}*/

void main()
{
    Light light = lights[0];
    //vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 color = vec3(1.0);//texture(materialMaps.diffuseMap, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = light.color;
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    //vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 lightDir = vec3(1.0);
    if(shadowMapping) lightDir = normalize(lightPos - fs_in.FragPos);
    else lightDir = normalize(light.position - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;
    // calculate shadow
    float shadow = 0.0;
    if(shadowMapping) shadow = ShadowCalculation(fs_in.FragPos, normal, lightDir);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    FragColor = vec4(lighting, 1.0);
}