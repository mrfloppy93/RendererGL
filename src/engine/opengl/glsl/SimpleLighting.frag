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
uniform bool shadowMapping;
uniform sampler2D shadowMap[16];
uniform mat4 lightSpaceMatrices[16];
uniform float cascadePlaneDistances[16];
uniform int cascadeCount; // number of frusta - 1

uniform vec3 lightPos;

uniform Light lights[MAX_LIGHTS];
uniform int nLights;

uniform vec3 viewPos;
uniform mat4 view;
uniform float farPlane;

int getCascadeLayer(vec3 fragPosWorldSpace) {
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
    return layer;
}

float ShadowCalculation(vec3 fragPosWorldSpace, vec3 normal, vec3 lightDir)
{
    int layer = getCascadeLayer(fragPosWorldSpace);

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)

    float closestDepth;

    switch(layer) {
        case 0: {
                    closestDepth = texture(shadowMap[0], projCoords.xy).r;
                    break;
                }
        case 1: {
                    closestDepth = texture(shadowMap[1], projCoords.xy).r;
                    break;
                }
        case 2: {
                    closestDepth = texture(shadowMap[2], projCoords.xy).r;
                    break;
                }
        case 3: {
                    closestDepth = texture(shadowMap[3], projCoords.xy).r;
                    break;
                }
        case 4: {
                    closestDepth = texture(shadowMap[4], projCoords.xy).r;
                    break;
                }
        case 5: {
                    closestDepth = texture(shadowMap[5], projCoords.xy).r;
                    break;
                }
        default:{
                    closestDepth = texture(shadowMap[0], projCoords.xy).r;
                    break;
                }
    }
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    if (currentDepth > 1.0)
    {
        return 0.0;
    }

    // check whether current frag pos is in shadow
    float bias = max(0.03 * (1.0 - dot(normal, lightDir)), 0.005);
    // calculate bias (based on depth map resolution and slope)
    const float biasModifier = 0.9f;
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
}

void main()
{
    Light light = lights[0];
    //vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 color;//texture(materialMaps.diffuseMap, fs_in.TexCoords).rgb;

    switch(getCascadeLayer(fs_in.FragPos)) {
    case 0: {
            color = vec3(1.0,0.5,0.5);
            break;
            }
    case 1: {
            color = vec3(0.5,1.0,0.5);
            break;
            }
    case 2: {
            color = vec3(0.5,0.5,1.0);
            break;
            }
    case 3: {
            color = vec3(0.8,0.8,0.0);
            break;
            }
    case 4: {
            color = vec3(0.8,0.0,0.8);
            break;
            }
    case 5: {
            color = vec3(0.0,0.8,0.8);
            break;
            }
    default:{
            color = vec3(1.0);
            break;
            }
    }

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