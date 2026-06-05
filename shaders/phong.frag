#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float shininess;
uniform float shadowStrength;
uniform float specularStrength;
uniform float ambientStrength;
uniform float aiStrength;
uniform float materialFactor;
uniform bool useBlinn;
uniform bool enableAIGI;
uniform int lightingMode;
uniform bool enableShadow;
uniform bool enableAttenuation;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) {
        return 0.0;
    }

    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    float shadow = 0.0;
    for (int x = -2; x <= 2; ++x) {
        for (int y = -2; y <= 2; ++y) {
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 25.0;
}

vec3 neuralGI(vec3 normal, vec3 lightDir, vec3 objectColor, float shadow, float materialFactor)
{
    float upward = max(normal.y, 0.0);
    float backFace = max(dot(normal, -lightDir), 0.0);
    float shadowBounce = smoothstep(0.15, 1.0, shadow);
    float materialBounce = mix(0.65, 1.20, materialFactor);

    vec3 baseBounce = 0.08 * objectColor;
    vec3 skyFloorBounce = 0.05 * vec3(0.80, 0.75, 0.68) * upward;
    vec3 shadowFill = 0.12 * objectColor * shadowBounce;
    vec3 grazingFill = 0.04 * objectColor * backFace;

    return clamp((baseBounce + skyFloorBounce + shadowFill + grazingFill) * materialBounce, 0.0, 1.0);
}

void main()
{
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float spec = 0.0;
    if (diff > 0.0) {
        if (useBlinn) {
            vec3 halfwayDir = normalize(lightDir + viewDir);
            spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
        } else {
            vec3 reflectDir = reflect(-lightDir, norm);
            spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        }
    }
    vec3 specular = specularStrength * spec * lightColor;

    if (enableAttenuation) {
        float d = length(lightPos - FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * d + 0.032 * d * d);
        diffuse *= attenuation;
        specular *= attenuation;
        ambient *= mix(0.7, 1.0, attenuation);
    }

    float shadow = enableShadow ? ShadowCalculation(FragPosLightSpace, norm, lightDir) : 0.0;
    float shadowFactor = 1.0 - shadow * shadowStrength;

    vec3 lighting;
    if (lightingMode == 1) {
        lighting = ambient;
    } else if (lightingMode == 2) {
        lighting = shadowFactor * diffuse;
    } else if (lightingMode == 3) {
        lighting = shadowFactor * specular;
    } else {
        lighting = ambient + shadowFactor * (diffuse + specular);
    }

    vec3 result = lighting * objectColor;
    if (enableAIGI) {
        result += aiStrength * neuralGI(norm, lightDir, objectColor, shadow, materialFactor);
    }

    FragColor = vec4(clamp(result, 0.0, 1.0), 1.0);
}
