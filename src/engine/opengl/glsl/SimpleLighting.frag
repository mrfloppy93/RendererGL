#version 330 core

#define MAX_LIGHTS 64
#define MAX_TEXTURES 64

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

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

in vec3 ourColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
in vec4 FragPosLightSpace;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

out vec4 FragColor;

uniform vec3 viewPos;
uniform Material material;

uniform Light lights[MAX_LIGHTS];
uniform int nLights;

uniform MaterialMaps materialMaps;
uniform bool hasDiffuse;
uniform bool hasSpecular;
uniform bool hasEmission;
uniform bool hasNormalMap;
uniform bool hasDepthMap;
uniform float emissionStrength;

uniform bool blinn;

uniform float heightScale;

uniform bool shadowMapping;
uniform sampler2D shadowMap[16];
uniform mat4 lightSpaceMatrices[16];
uniform float cascadePlaneDistances[16];
uniform int cascadeCount; // number of frusta - 1
uniform vec3 lightPos;
uniform float farPlane;
uniform mat4 view;

vec2 texCoord = TexCoord;

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

vec4 calculateAmbient(Light light) {
    vec4 ambient = vec4(1.0);

    vec4 lightAmbient = vec4(light.ambient, 1.0);
    vec4 lightColor = vec4(light.color, 1.0);

    if(hasDiffuse) {
        vec4 textureDiffuse = texture(materialMaps.diffuseMap, texCoord);
        ambient = lightAmbient * lightColor * textureDiffuse;
    }
    else ambient = lightAmbient * lightColor * vec4(ourColor, 1.0);

    return ambient;
}

vec4 calculateDiffuse(Light light, vec3 normal, vec3 lightDir) {
    vec4 diffuse = vec4(1.0);
    float diff = max(dot(normal, lightDir), 0.0);

    vec4 lightDiffuse = vec4(light.diffuse, 1.0);
    vec4 materialDiffuse = vec4(material.diffuse, 1.0);

    if(hasDiffuse) {
        vec4 textureDiffuse = texture(materialMaps.diffuseMap, texCoord);
        diffuse = lightDiffuse * diff * textureDiffuse;
    }else diffuse = lightDiffuse * (diff * materialDiffuse);

    return diffuse;
}

vec4 calculateSpecular(Light light, vec3 normal, vec3 lightDir) {
    vec4 specular = vec4(1.0);
    vec3 viewDir = normalize(viewPos - FragPos);

    if(hasNormalMap) viewDir = normalize(TangentViewPos - TangentFragPos);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;

    vec4 lightSpecular = vec4(light.specular, 1.0);
    vec4 materialSpecular = vec4(material.specular, 1.0);

    if(blinn) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    } else spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    if(hasSpecular) {
        vec4 specularMapInfo = texture(materialMaps.specularMap, texCoord);
        specular = lightSpecular * spec * specularMapInfo;
    }else specular = lightSpecular * (spec * materialSpecular);

    return specular;
}

vec4 calculateEmission() {
    vec4 emission = vec4(0.0);
    if(hasEmission) emission = texture(materialMaps.emissionMap, texCoord);
    return emission * emissionStrength;
}

float calculateShadow(vec3 fragPosWorldSpace) {
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
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
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
}

vec4 getLightColor(Light light) {

    // Calculate ambient, diffuse and specular
    vec3 norm = normalize(Normal);

    vec3 lightDir = normalize(light.position - FragPos);

    if(hasNormalMap) {
        norm = texture(materialMaps.normalMap, texCoord).rgb;
        // transform normal vector to range [-1,1]
        norm = normalize(norm * 2.0 - 1.0);  // this normal is in tangent space
        lightDir = normalize(TangentLightPos - TangentFragPos);
    }

    vec4 ambient = calculateAmbient(light);
    vec4 diffuse = calculateDiffuse(light, norm, lightDir);
    vec4 specular = calculateSpecular(light, norm, lightDir);

    // Calculate emission
    vec4 emission = calculateEmission();

    // Calculate attenuation
    if(light.pointLight) {
        float distance = length(light.position - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        ambient  *= attenuation;
        diffuse  *= attenuation;
        specular *= attenuation;
    }

    // Calculate shadow
    float shadow = 0.0;
    if(shadowMapping) shadow = calculateShadow(FragPos);

    vec4 lightColor = vec4(light.color, 1.0);

    switch(getCascadeLayer(FragPos)) {
        case 0: {
            lightColor += vec4(1.0,0.5,0.5,1.0);
            break;
        }
        case 1: {
            lightColor += vec4(0.5,1.0,0.5,1.0);
            break;
        }
        case 2: {
            lightColor += vec4(0.5,0.5,1.0,1.0);
            break;
        }
        case 3: {
            lightColor += vec4(0.8,0.8,0.0,1.0);
            break;
        }
        case 4: {
            lightColor += vec4(0.8,0.0,0.8,1.0);
            break;
        }
        case 5: {
            lightColor += vec4(0.0,0.8,0.8,1.0);
            break;
        }
        default:{
            lightColor += vec4(1.0);
            break;
        }
    }


    return (ambient + (1.0 - shadow) * (diffuse + specular) + emission) * lightColor;
}

vec2 parallaxMapping(vec2 texCoords, vec3 viewDir) {

    if(!hasDepthMap)
    return texCoords;

    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));

    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;

    // depth of current layer
    float currentLayerDepth = 0.0;

    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * heightScale;
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(materialMaps.depthMap, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue) {

        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;

        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(materialMaps.depthMap, currentTexCoords).r;

        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(materialMaps.depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main() {



    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    texCoord = parallaxMapping(texCoord, viewDir);

    vec4 lightColor = vec4(0.0);
    for(int i = 0; i < nLights; i ++) lightColor += getLightColor(lights[i]);

    // Apply transparency for blending
    if(hasDiffuse) {
        vec4 textureDiffuse = texture(materialMaps.diffuseMap, texCoord);
        lightColor.a = textureDiffuse.a;
    }

    FragColor = lightColor;
}