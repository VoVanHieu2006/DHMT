from pathlib import Path

import numpy as np


BASE_DIR = Path(__file__).resolve().parent
WEIGHTS_PATH = BASE_DIR / "neural_gi_weights.npz"
OUTPUT_PATH = BASE_DIR.parent / "shaders" / "neural_gi.glsl"


def float_array(name, values):
    flat = values.reshape(-1)
    body = ", ".join(f"{float(v):.8f}" for v in flat)
    return f"const float {name}[{len(flat)}] = float[{len(flat)}]({body});"


def main():
    if not WEIGHTS_PATH.exists():
        print("No trained weights found; exporting the built-in educational approximation.")
        OUTPUT_PATH.write_text(FALLBACK_GLSL, encoding="utf-8")
        print(f"Saved {OUTPUT_PATH}")
        return

    weights = np.load(WEIGHTS_PATH)
    print("Loaded trained arrays:")
    for name in ["w1", "b1", "w2", "b2", "w3", "b3"]:
        print(f"  {name}: {weights[name].shape}")

    glsl = [
        "// AI-GI Lite exported GLSL from ai/neural_gi_weights.npz.",
        "// Copy this neuralGI function into phong.frag if you want to replace the built-in compact approximation.",
        float_array("AI_W1", weights["w1"]),
        float_array("AI_B1", weights["b1"]),
        float_array("AI_W2", weights["w2"]),
        float_array("AI_B2", weights["b2"]),
        float_array("AI_W3", weights["w3"]),
        float_array("AI_B3", weights["b3"]),
        """
float aiRelu(float x)
{
    return max(x, 0.0);
}

float aiSigmoid(float x)
{
    return 1.0 / (1.0 + exp(-x));
}

vec3 neuralGI(vec3 normal, vec3 lightDir, vec3 objectColor, float shadow, float materialFactor)
{
    float x[11] = float[11](
        normal.x, normal.y, normal.z,
        lightDir.x, lightDir.y, lightDir.z,
        objectColor.r, objectColor.g, objectColor.b,
        shadow, materialFactor
    );

    float h1[16];
    for (int i = 0; i < 16; ++i) {
        float sumValue = AI_B1[i];
        for (int j = 0; j < 11; ++j) {
            sumValue += AI_W1[i * 11 + j] * x[j];
        }
        h1[i] = aiRelu(sumValue);
    }

    float h2[16];
    for (int i = 0; i < 16; ++i) {
        float sumValue = AI_B2[i];
        for (int j = 0; j < 16; ++j) {
            sumValue += AI_W2[i * 16 + j] * h1[j];
        }
        h2[i] = aiRelu(sumValue);
    }

    vec3 outColor = vec3(0.0);
    for (int i = 0; i < 3; ++i) {
        float sumValue = AI_B3[i];
        for (int j = 0; j < 16; ++j) {
            sumValue += AI_W3[i * 16 + j] * h2[j];
        }
        outColor[i] = aiSigmoid(sumValue);
    }

    return clamp(outColor, 0.0, 1.0);
}
""",
    ]
    OUTPUT_PATH.write_text("\n".join(glsl), encoding="utf-8")
    print(f"Saved learned GLSL weights to {OUTPUT_PATH}")


FALLBACK_GLSL = """// AI-GI Lite exported GLSL.
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
"""


if __name__ == "__main__":
    main()
