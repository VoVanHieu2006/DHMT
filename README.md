# PhongLighting

Project mô phỏng ánh sáng và đổ bóng bằng OpenGL 3.3 Core Profile. Chương trình hỗ trợ Phong, Blinn-Phong, PBR Cook-Torrance cơ bản, Shadow Mapping, Global Illumination Approximation, AI-GI Lite, material presets và bảng điều khiển native OpenGL bên phải cửa sổ.

Runtime C++ chỉ dùng OpenGL, GLFW, GLEW, GLM, CMake và Ninja. Module AI dùng Python để train offline và export sang GLSL; không nhúng PyTorch hay ONNX Runtime vào C++.

## 1. Cài đặt MSYS2 UCRT64

```bash
pacman -Syu
```

Mở lại terminal MSYS2 UCRT64 nếu được yêu cầu, rồi chạy:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-glfw mingw-w64-ucrt-x86_64-glew mingw-w64-ucrt-x86_64-glm
```

## 2. Build

```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## 3. Run

```bash
./build/PhongLighting.exe
```

Cửa sổ mặc định là `1600x900`. Nếu màn hình nhỏ hơn, app dùng khoảng 90% kích thước monitor. Scene nằm bên trái, panel điều khiển nằm cố định bên phải.

## 4. Render Modes

- Phong: ambient + diffuse + specular, specular dùng reflect vector.
- Blinn-Phong: ambient + diffuse + specular, specular dùng halfway vector.
- PBR: Cook-Torrance BRDF cơ bản với `metallic`, `roughness`, `ao`, direct point light, attenuation và shadow.

Shadow chỉ giảm direct light. Ambient, GI approximation và AI-GI indirect vẫn còn để vùng tối không bị đen tuyệt đối.

## 5. Native OpenGL UI

Panel bên phải được vẽ trực tiếp bằng OpenGL:

- Buttons: Phong, Blinn, PBR, material preset, reset.
- Checkboxes: Ambient, Diffuse, Specular, Shadow, GI, AI-GI, Orbit, Attenuation.
- Sliders: Shadow Strength, GI Strength, Color Bleed, Bounce Light, Metallic, Roughness, AO, AI Strength, Light Intensity.
- Sidebar rộng tối thiểu 420px, hoặc khoảng 28% chiều rộng cửa sổ khi cửa sổ lớn.
- Sidebar responsive theo công thức `clamp(framebufferWidth * 0.26, 420, 560)`.
- Mouse hit-test được quy đổi đúng theo framebuffer để tránh lệch click khi resize/DPI scaling.
- Nếu panel dài hơn chiều cao cửa sổ, đưa chuột vào sidebar và cuộn mouse wheel.
- Text trong sidebar đã được tăng scale: header, section title, label, slider value và material name đều lớn hơn để dễ đọc.

Không dùng web UI, localhost, HTML/CSS/JS hoặc ImGui.

## 6. Controls

- ESC: thoát.
- F11: bật/tắt fullscreen, thoát fullscreen quay lại windowed mode `1600x900`.
- P: Phong.
- B: Blinn-Phong.
- V: PBR.
- 1: Ambient only.
- 2: Diffuse only.
- 3: Specular only.
- 4: Full lighting.
- O: bật/tắt Shadow Mapping.
- A: bật/tắt Attenuation.
- G: bật/tắt Global Illumination Approx.
- I: bật/tắt AI-GI Lite.
- T: bật/tắt Auto Orbit Light.
- M: đổi material preset.
- C: chọn Cube.
- H: chọn Sphere. Dùng `H` vì `S` đã dùng cho camera backward.
- Y: chọn Pyramid.
- = hoặc keypad +: thêm object theo type đang chọn.
- Delete: xóa object đang chọn.
- N: random màu object đang chọn.
- R: reset scene.
- Q/E: giảm/tăng shininess.
- [/]: giảm/tăng shadowStrength.
- K/L: giảm/tăng aiStrength.
- Arrow keys: di chuyển light theo X/Y.
- Z/X: di chuyển light theo Z.
- W/S: di chuyển camera gần/xa.

## 7. Scene

- Room kiểu Cornell-box/studio lab với kích thước khoảng `12 x 10 x 6`.
- Floor có grid procedural và khít với tường.
- Left wall màu đỏ, khít với back wall.
- Right wall màu xanh, khít với back wall.
- Back wall màu xám tối.
- Ceiling màu tối hơn, khép góc trên của phòng.
- Side walls được rút ngắn chiều sâu về phía camera để back wall và bố cục trung tâm thoáng hơn, nhưng vẫn giữ góc sau khít với room.
- Cube bên trái.
- Sphere bên phải.
- Lamp cube màu vàng nhạt.
- Shadow đổ lên floor.

Khi bật GI, vùng tối sáng hơn nhẹ và có color bleeding từ tường đỏ/xanh.

## 8. Material Presets

- Plastic: metallic 0.0, roughness 0.45, shininess 32.
- Rubber: metallic 0.0, roughness 0.85, shininess 8.
- Metal-like: metallic 0.8, roughness 0.25, shininess 128.
- Ceramic: metallic 0.0, roughness 0.30, shininess 64.

Nhấn `M` hoặc nút `NEXT MATERIAL` trên UI để đổi preset.

## 9. Object Selection

Sidebar có section `OBJECT SELECTION`:

- Chọn mode `SINGLE` hoặc `MULTIPLE`.
- Button type dùng dạng cycle: `Cube -> Sphere -> Pyramid`.
- `+` hoặc `ADD`: thêm object theo type đang chọn.
- `RANDOM`: đổi màu object đang chọn.
- `REMOVE`: xóa object đang chọn.
- `CLEAR`: tạo lại scene mặc định gồm Cube, Sphere, Pyramid.
- Danh sách object cho phép chọn object hiện tại và bật/tắt visible.

Mặc định app mở ở Multiple mode với 3 object: Cube màu cam, Sphere màu teal và Pyramid màu vàng.

## 10. AI-GI Lite

AI-GI Lite là một Neural Indirect Lighting Approximation nhỏ:

1. Generate synthetic dataset.
2. Train MLP bằng PyTorch.
3. Export weights sang GLSL.
4. Shader cộng `neuralGI(...)` vào ánh sáng gián tiếp.

Python optional:

```bash
pip install numpy torch
python ai/generate_dataset.py
python ai/train_neural_gi.py
python ai/export_glsl_weights.py
```

Runtime C++ không cần Python. Xem thêm `ai/README_AI.md`.

## 11. Gợi ý ảnh báo cáo

- Phong + Shadow.
- Blinn-Phong + Shadow.
- PBR roughness thấp.
- PBR roughness cao.
- GI OFF.
- GI ON.
- AI-GI OFF.
- AI-GI ON.
- Shadow OFF.
- Shadow ON.
- UI panel tổng thể.
- Material Plastic/Rubber/Metal-like/Ceramic.

## 12. Ghi chú kỹ thuật

- PBR thật trong project: Cook-Torrance BRDF trực tiếp trong `phong.frag`.
- GI approximation: công thức heuristic trong shader, không phải path tracing/radiosity.
- AI-GI Lite: approximation giáo dục, train offline bằng Python và dùng GLSL ở runtime.
- Nếu shadow bị acne, chỉnh bias trong `ShadowCalculation`.
- Nếu PBR quá sáng/tối, chỉnh `Light Intensity`, `Roughness`, `Metallic`, `AO`.
