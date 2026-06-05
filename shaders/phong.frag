#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float shininess;
uniform bool useBlinn;
uniform int lightingMode;

void main()
{
    // 1. Ambient: basic indirect light approximation
    float ambientStrength = 0.10;
    vec3 ambient = ambientStrength * lightColor;

    // 2. Diffuse: Lambert lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 3. Specular: Phong or Blinn-Phong
    float specularStrength = 0.50;
    vec3 viewDir = normalize(viewPos - FragPos);

    float spec = 0.0;

    if (useBlinn) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    } else {
        vec3 reflectDir = reflect(-lightDir, norm);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    }

    vec3 specular = specularStrength * spec * lightColor;

    vec3 lighting;
    if (lightingMode == 1) {
        lighting = ambient;
    } else if (lightingMode == 2) {
        lighting = ambient + diffuse;
    } else if (lightingMode == 3) {
        lighting = ambient + diffuse + specular;
    } else {
        lighting = ambient + diffuse + specular;
    }

    vec3 result = lighting * objectColor;
    FragColor = vec4(result, 1.0);
}
