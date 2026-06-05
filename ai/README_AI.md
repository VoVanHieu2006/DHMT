# AI-GI Lite

AI-GI Lite, hay Neural Indirect Lighting Approximation, là một module giáo dục để minh họa AI-assisted lighting trong project OpenGL. Nó dùng một MLP nhỏ để ước lượng ánh sáng gián tiếp nhẹ từ normal, light direction, object color, shadow và material factor.

Đây không phải NeRF full, không phải Neural Radiance Caching full và không thay thế Phong/Blinn-Phong hay Shadow Mapping. Nó chỉ bổ sung một thành phần indirect light để vùng shadow bớt tối tuyệt đối và dễ so sánh Traditional Rendering với AI-assisted Lighting.

## Pipeline

1. Generate dataset tổng hợp bằng công thức indirect lighting đơn giản.
2. Train MLP nhỏ bằng PyTorch.
3. Export weights sang GLSL.
4. Fragment shader gọi `neuralGI(...)`.
5. So sánh render truyền thống với AI-GI OFF và AI-assisted với AI-GI ON.

## Lệnh chạy

```bash
python ai/generate_dataset.py
python ai/train_neural_gi.py
python ai/export_glsl_weights.py
```

Python dependencies:

```bash
pip install numpy torch
```

Sau khi export GLSL:

```bash
cmake --build build
./build/PhongLighting.exe
```

Ghi chú: OpenGL runtime không cần PyTorch, ONNX Runtime hay thư viện ML C++ nào. PyTorch chỉ dùng offline để train.

## Controls

- I: bật/tắt AI-GI Lite.
- K/L: giảm/tăng AI strength.

## Gợi ý ảnh báo cáo

- Traditional: Phong + Shadow Mapping, AI-GI OFF.
- AI-assisted: Phong + Shadow Mapping + AI-GI ON.
- AI strength thấp.
- AI strength cao.
- Shadow ON với AI-GI OFF.
- Shadow ON với AI-GI ON.
