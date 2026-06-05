#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform int renderMode;
uniform float shininess;
uniform float shadowStrength;
uniform float specularStrength;
uniform float ambientStrength;
uniform float metallic;
uniform float roughness;
uniform float ao;
uniform float lightIntensity;
uniform float giStrength;
uniform float colorBleedingStrength;
uniform float bounceStrength;
uniform float aiStrength;
uniform float materialFactor;
uniform bool enableAmbient;
uniform bool enableDiffuse;
uniform bool enableSpecular;
uniform bool enableGI;
uniform bool enableAIGI;
uniform int lightingMode;
uniform bool enableShadow;
uniform bool enableAttenuation;
uniform bool isFloor;
uniform vec3 floorColor;
uniform vec3 leftWallColor;
uniform vec3 rightWallColor;
uniform vec3 backWallColor;
uniform sampler2D shadowMap;

const float PI = 3.14159265359;

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

float DistributionGGX(vec3 N, vec3 H, float roughnessValue)
{
    float a = roughnessValue * roughnessValue;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom + 0.0001);
}

float GeometrySchlickGGX(float NdotV, float roughnessValue)
{
    float r = roughnessValue + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k + 0.0001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughnessValue)
{
    float ggx1 = GeometrySchlickGGX(max(dot(N, V), 0.0), roughnessValue);
    float ggx2 = GeometrySchlickGGX(max(dot(N, L), 0.0), roughnessValue);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 computeApproxGI(vec3 fragPos, vec3 normal, vec3 baseColor, float shadow)
{
    float up = normal.y * 0.5 + 0.5;
    vec3 hemi = mix(floorColor, vec3(0.45, 0.50, 0.60), up);
    vec3 floorBounce = floorColor * max(normal.y, 0.0) * bounceStrength;

    float roomHalfWidth = 4.0;
    float leftInfluence = smoothstep(4.0, 0.0, abs(fragPos.x + roomHalfWidth));
    float rightInfluence = smoothstep(4.0, 0.0, abs(fragPos.x - roomHalfWidth));
    float backInfluence = smoothstep(5.0, 0.0, abs(fragPos.z + 4.0));
    vec3 bleeding = leftWallColor * leftInfluence + rightWallColor * rightInfluence + backWallColor * backInfluence;
    vec3 shadowFill = baseColor * shadow * 0.10 * bounceStrength;

    vec3 gi = 0.08 * hemi + 0.10 * floorBounce + colorBleedingStrength * 0.08 * bleeding + shadowFill;
    return clamp(giStrength * gi * baseColor, 0.0, 1.0);
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    float d = length(lightPos - FragPos);
    float attenuation = enableAttenuation ? 1.0 / (1.0 + 0.09 * d + 0.032 * d * d) : 1.0;
    float shadow = enableShadow ? ShadowCalculation(FragPosLightSpace, norm, lightDir) : 0.0;
    float shadowFactor = 1.0 - shadow * shadowStrength;

    vec3 color = vec3(0.0);

    if (renderMode == 2) {
        vec3 albedo = objectColor;
        vec3 V = viewDir;
        vec3 L = lightDir;
        vec3 H = normalize(V + L);
        vec3 radiance = lightColor * lightIntensity * attenuation;

        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        float NDF = DistributionGGX(norm, H, clamp(roughness, 0.04, 1.0));
        float G = GeometrySmith(norm, V, L, clamp(roughness, 0.04, 1.0));
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(norm, V), 0.0) * max(dot(norm, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;
        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
        float NdotL = max(dot(norm, L), 0.0);
        vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;
        vec3 ambient = vec3(0.03) * albedo * ao;

        color = ambient + shadowFactor * Lo;
        color = color / (color + vec3(1.0));
        color = pow(color, vec3(1.0 / 2.2));
    } else {
        vec3 ambient = enableAmbient ? ambientStrength * lightColor : vec3(0.0);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = enableDiffuse ? diff * lightColor : vec3(0.0);

        float spec = 0.0;
        if (diff > 0.0 && enableSpecular) {
        if (renderMode == 1) {
            vec3 halfwayDir = normalize(lightDir + viewDir);
            spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
        } else {
            vec3 reflectDir = reflect(-lightDir, norm);
            spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        }
        }
        vec3 specular = specularStrength * spec * lightColor;

        diffuse *= attenuation;
        specular *= attenuation;
        ambient *= mix(0.7, 1.0, attenuation);

    if (lightingMode == 1) {
            color = ambient * objectColor;
    } else if (lightingMode == 2) {
            color = shadowFactor * diffuse * objectColor;
    } else if (lightingMode == 3) {
            color = shadowFactor * specular * objectColor;
    } else {
            color = (ambient + shadowFactor * (diffuse + specular)) * objectColor;
        }
    }

    vec3 result = color;
    if (enableGI) {
        result += computeApproxGI(FragPos, norm, objectColor, shadow);
    }
    if (enableAIGI) {
        result += aiStrength * neuralGI(norm, lightDir, objectColor, shadow, materialFactor);
    }

    if (isFloor) {
        vec2 gridCoord = abs(fract(FragPos.xz * 1.5 - 0.5) - 0.5) / fwidth(FragPos.xz * 1.5);
        float grid = 1.0 - min(min(gridCoord.x, gridCoord.y), 1.0);
        result = mix(result, result + vec3(0.08, 0.10, 0.10), grid * 0.35);
    }

    FragColor = vec4(clamp(result, 0.0, 1.0), 1.0);
}
