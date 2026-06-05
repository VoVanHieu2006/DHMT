# PhongLighting

Demo mô phỏng ánh sáng và đổ bóng bằng OpenGL 3.3 Core Profile. Project hỗ trợ Phong, Blinn-Phong, Shadow Mapping, attenuation, auto-orbit light, material presets và AI-GI Lite để minh họa ánh sáng gián tiếp bằng AI ở mức giáo dục.

Phần PBR và Global Illumination đầy đủ được dùng ở mức lý thuyết, báo cáo và hướng phát triển. Neural Rendering được tích hợp nhẹ qua AI-GI Lite: Python train offline, export sang GLSL, runtime C++ vẫn chỉ dùng OpenGL/GLFW/GLEW/GLM.

## 1. Cài đặt MSYS2 UCRT64

Mở terminal **MSYS2 UCRT64** và cập nhật hệ thống:

```bash
pacman -Syu
```

Nếu MSYS2 yêu cầu đóng terminal, hãy đóng và mở lại **MSYS2 UCRT64**, sau đó cài toolchain và thư viện:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-glfw mingw-w64-ucrt-x86_64-glew mingw-w64-ucrt-x86_64-glm
```

Project dùng:

- GLFW: tạo cửa sổ, OpenGL context và xử lý input.
- GLEW: nạp các hàm OpenGL hiện đại.
- GLM: tính toán vector và ma trận.
- CMake + Ninja: cấu hình và build project.

## 2. Build

Trong thư mục project:

```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

CMake build executable `PhongLighting` và copy toàn bộ thư mục `shaders/` vào thư mục chứa file `.exe`.

## 3. Run

Chạy trong MSYS2 UCRT64:

```bash
./build/PhongLighting.exe
```

## 4. Controls

- ESC: thoát.
- Arrow keys: di chuyển nguồn sáng theo X/Y.
- Z/X: di chuyển nguồn sáng theo Z.
- W/S: di chuyển camera gần/xa.
- Q/E: giảm/tăng shininess.
- P: Phong.
- B: Blinn-Phong.
- 1: Ambient only.
- 2: Diffuse only.
- 3: Specular only.
- 4: Full lighting.
- O: bật/tắt Shadow Mapping.
- A: bật/tắt Attenuation.
- T: bật/tắt Auto Orbit Light.
- R: reset scene.
- M: đổi material preset.
- [/]: giảm/tăng shadowStrength.
- I: bật/tắt AI-GI Lite.
- K/L: giảm/tăng AI strength.

Title bar hiển thị trạng thái dạng:

```text
Phong | Shininess: 32 | Lighting: Full | Shadow: ON(0.65) | AI-GI: ON(0.35) | Attenuation: ON | Orbit: OFF | Material: Plastic
```

## 5. Nội dung demo

- Floor lớn ở `y = -1.0`, nhận bóng đổ khi bật Shadow Mapping.
- Cube bên trái, sphere bên phải, lamp cube biểu diễn vị trí point light.
- Lamp cube màu vàng nhạt, di chuyển theo `lightPos` khi bật auto-orbit.
- Phong dùng `reflectDir` và `dot(viewDir, reflectDir)`.
- Blinn-Phong dùng `halfwayDir` và `dot(norm, halfwayDir)`.
- Lighting modes:
  - `1`: Ambient only.
  - `2`: Diffuse only.
  - `3`: Specular only.
  - `4`: Full lighting.
- Shadow Mapping:
  - Render depth map từ góc nhìn nguồn sáng.
  - Dùng `lightSpaceMatrix`.
  - Có bias giảm shadow acne.
  - Có PCF 5x5 để bóng mềm hơn.
  - `shadowStrength` mặc định `0.65` để bóng không đen tuyệt đối.
- Attenuation:
  - `1.0 / (constant + linear * d + quadratic * d * d)`
  - `constant = 1.0`
  - `linear = 0.09`
  - `quadratic = 0.032`

## 6. Material Presets

Nhấn `M` để chuyển preset:

- Plastic: shininess 32, màu cam/teal, specular vừa.
- Rubber: shininess 8, màu tối hơn, specular yếu.
- Metal-like: shininess 128, màu xám/bạc, specular mạnh.
- Ceramic: shininess 64, màu sáng, specular vừa.

Các preset không phải PBR đầy đủ; chúng dùng uniform màu, shininess, ambientStrength và specularStrength để demo ảnh hưởng vật liệu trong mô hình Phong/Blinn-Phong.

## 7. AI-assisted Lighting / AI-GI Lite

Project có thêm module AI nhỏ để ước lượng ánh sáng gián tiếp. AI-GI không thay thế Phong/Blinn-Phong và không thay thế Shadow Mapping; nó chỉ cộng thêm một lượng indirect light nhẹ để vùng tối/shadow có cảm giác bounce light.

Pipeline AI:

1. `python ai/generate_dataset.py`: sinh dataset tổng hợp.
2. `python ai/train_neural_gi.py`: train MLP nhỏ bằng PyTorch.
3. `python ai/export_glsl_weights.py`: export weights sang `shaders/neural_gi.glsl`.
4. Build và chạy lại C++.

Python dependencies:

```bash
pip install numpy torch
```

Nếu không train lại AI, project vẫn chạy với hàm `neuralGI(...)` tích hợp sẵn trong `phong.frag`. Runtime OpenGL không cần PyTorch.

Controls AI:

- `I`: bật/tắt AI-GI Lite.
- `K/L`: giảm/tăng `aiStrength`.

Xem thêm: `ai/README_AI.md`.

## 8. Gợi ý chụp ảnh báo cáo

- Full scene với cube + sphere + floor + shadow.
- Ambient only.
- Diffuse only.
- Specular only.
- Full Phong.
- Full Blinn-Phong.
- Shininess thấp.
- Shininess cao.
- Shadow OFF.
- Shadow ON.
- Attenuation OFF.
- Attenuation ON.
- Material Plastic.
- Material Rubber.
- Material Metal-like.
- Material Ceramic.
- Traditional: Shadow Mapping, AI-GI OFF.
- AI-assisted: Shadow Mapping, AI-GI ON.
- AI strength thấp.
- AI strength cao.

## 9. Lỗi OpenGL/shadow thường gặp

- Shader không load: chạy executable từ project root hoặc kiểm tra thư mục `build/shaders/` đã được copy sau build.
- Bóng bị răng cưa: tăng kích thước shadow map hoặc tăng vùng PCF, đổi lại sẽ tốn hiệu năng hơn.
- Shadow acne: tăng bias trong `ShadowCalculation`.
- Peter-panning, bóng tách khỏi vật thể: giảm bias.
- Không thấy lamp cube: tắt orbit bằng `T`, reset bằng `R`, hoặc di chuyển light bằng arrow/Z/X.
- AI-GI quá sáng: giảm bằng `K` hoặc tắt bằng `I`.
