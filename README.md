# PhongLighting

Demo Phong / Blinn-Phong Lighting bằng OpenGL 3.3 Core Profile. Project mô phỏng ánh sáng thời gian thực với cube, sphere, floor và lamp object, có Shadow Mapping cơ bản để tạo bóng đổ lên mặt phẳng.

Người dùng có thể thay đổi vị trí nguồn sáng, vị trí camera, shininess, chế độ Phong/Blinn-Phong, các thành phần ambient/diffuse/specular, attenuation và Shadow Mapping.

## 1. Cài đặt MSYS2 UCRT64

Cài MSYS2, mở terminal **MSYS2 UCRT64**, rồi cập nhật hệ thống:

```bash
pacman -Syu
```

Nếu MSYS2 yêu cầu đóng terminal, hãy đóng và mở lại **MSYS2 UCRT64**, sau đó cài công cụ build và thư viện:

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

File chạy sau khi build:

```bash
build/PhongLighting.exe
```

CMake sẽ copy toàn bộ thư mục `shaders/` vào thư mục chứa file `.exe`.

## 3. Run

Chạy trong MSYS2 UCRT64:

```bash
./build/PhongLighting.exe
```

## 4. Controls

- ESC: thoát.
- Arrow keys: di chuyển light theo X/Y.
- Z/X: di chuyển light theo Z.
- W/S: di chuyển camera gần/xa.
- Q/E: giảm/tăng shininess.
- P: Phong.
- B: Blinn-Phong.
- 1: Ambient only.
- 2: Diffuse only.
- 3: Specular only.
- 4: Full lighting.
- O: bật/tắt Shadow Mapping.
- A: bật/tắt attenuation.
- T: bật/tắt auto-orbit light.
- R: reset scene.

Tiêu đề cửa sổ hiển thị mode hiện tại: Phong/Blinn-Phong, shininess, lighting mode, Shadow ON/OFF, Attenuation ON/OFF và Orbit ON/OFF.

## 5. Nội dung demo

- Floor lớn ở `y = -1.0`, nhận bóng đổ khi bật Shadow Mapping.
- Cube màu cam ở bên trái.
- Sphere màu teal ở bên phải.
- Lamp cube nhỏ biểu diễn vị trí point light.
- Toggle Phong / Blinn-Phong:
  - Phong dùng `reflectDir` và `dot(viewDir, reflectDir)`.
  - Blinn-Phong dùng `halfwayDir` và `dot(norm, halfwayDir)`.
- Lighting modes:
  - `1`: chỉ ambient.
  - `2`: chỉ diffuse.
  - `3`: chỉ specular.
  - `4`: ambient + diffuse + specular.
- Shadow Mapping:
  - Render depth map từ góc nhìn nguồn sáng.
  - Dùng `lightSpaceMatrix`.
  - Có bias giảm shadow acne.
  - Có PCF 3x3 để bóng bớt răng cưa.
- Attenuation:
  - `1.0 / (constant + linear * d + quadratic * d * d)`
  - `constant = 1.0`
  - `linear = 0.09`
  - `quadratic = 0.032`

## 6. Gợi ý chụp ảnh báo cáo

- Full lighting.
- Ambient only.
- Diffuse only.
- Specular only.
- Phong.
- Blinn-Phong.
- Shadow OFF.
- Shadow ON.
- Shininess thấp.
- Shininess cao.
