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
uniform bool useBlinn;
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
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

void main()
{
    float ambientStrength = 0.12;
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
    vec3 specular = 0.5 * spec * lightColor;

    if (enableAttenuation) {
        float d = length(lightPos - FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * d + 0.032 * d * d);
        diffuse *= attenuation;
        specular *= attenuation;
        ambient *= mix(0.7, 1.0, attenuation);
    }

    float shadow = enableShadow ? ShadowCalculation(FragPosLightSpace, norm, lightDir) : 0.0;

    vec3 lighting;
    if (lightingMode == 1) {
        lighting = ambient;
    } else if (lightingMode == 2) {
        lighting = (1.0 - shadow) * diffuse;
    } else if (lightingMode == 3) {
        lighting = (1.0 - shadow) * specular;
    } else {
        lighting = ambient + (1.0 - shadow) * (diffuse + specular);
    }

    FragColor = vec4(lighting * objectColor, 1.0);
}
