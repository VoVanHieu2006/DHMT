from pathlib import Path

import numpy as np


OUTPUT_PATH = Path(__file__).with_name("neural_gi_dataset.npz")
SAMPLE_COUNT = 20000


def normalize(v):
    return v / np.maximum(np.linalg.norm(v, axis=1, keepdims=True), 1e-6)


def main():
    rng = np.random.default_rng(42)

    normal = normalize(rng.normal(size=(SAMPLE_COUNT, 3)).astype(np.float32))
    light_dir = normalize(rng.normal(size=(SAMPLE_COUNT, 3)).astype(np.float32))
    object_color = rng.uniform(0.05, 1.0, size=(SAMPLE_COUNT, 3)).astype(np.float32)
    shadow = rng.uniform(0.0, 1.0, size=(SAMPLE_COUNT, 1)).astype(np.float32)
    material_factor = rng.uniform(0.0, 1.0, size=(SAMPLE_COUNT, 1)).astype(np.float32)

    upward = np.maximum(normal[:, 1:2], 0.0)
    back_face = np.maximum(np.sum(normal * -light_dir, axis=1, keepdims=True), 0.0)
    material_bounce = 0.65 + (1.20 - 0.65) * material_factor

    indirect = 0.08 * object_color
    indirect += 0.05 * np.array([[0.80, 0.75, 0.68]], dtype=np.float32) * upward
    indirect += 0.12 * object_color * shadow
    indirect += 0.04 * object_color * back_face
    indirect *= material_bounce
    indirect = np.clip(indirect, 0.0, 1.0).astype(np.float32)

    inputs = np.concatenate([normal, light_dir, object_color, shadow, material_factor], axis=1)
    np.savez(OUTPUT_PATH, x=inputs.astype(np.float32), y=indirect)
    print(f"Saved {SAMPLE_COUNT} samples to {OUTPUT_PATH}")


if __name__ == "__main__":
    main()
