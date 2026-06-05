// AI-GI Lite exported GLSL.
// Runtime OpenGL does not need PyTorch. This function approximates learned
// indirect lighting from normal, light direction, albedo, shadow, and material.
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
