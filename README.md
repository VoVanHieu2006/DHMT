## 1. Cài công cụ cần thiết

Cài MSYS2 trước. MSYS2 là môi trường build native Windows và dùng pacman để cài thư viện C/C++ rất tiện. pacman -S <tên-gói> là cách cài package chính thức của MSYS2.

Mở app:

MSYS2 UCRT64

Chạy lần lượt:

```bash
pacman -Syu
```

Nếu nó yêu cầu đóng terminal thì đóng, mở lại MSYS2 UCRT64, rồi chạy tiếp:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-glfw mingw-w64-ucrt-x86_64-glew mingw-w64-ucrt-x86_64-glm
```

Các thư viện dùng trong project:

GLFW: tạo cửa sổ OpenGL, nhận input bàn phím.
GLEW: nạp các hàm OpenGL hiện đại.
GLM: tính toán vector, ma trận.
CMake + Ninja: build project.

GLFW là thư viện tạo window/context cho OpenGL, còn GLEW là extension loader; khi dùng GLEW thì cần include glew.h trước các header OpenGL/GLFW liên quan.


## 2. Clone project từ GitHub

Mở MSYS2 UCRT64, chuyển đến thư mục muốn lưu project, ví dụ Desktop:

```bash
cd /c/Users/Acer/Desktop
```

Clone project:

```bash
git clone https://github.com/VoVanHieu2006/DHMT.git
```

Đi vào thư mục project:

```bash
cd DHMT
```

## 3. Build project

Chạy lệnh cấu hình CMake:

```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug
```

Sau đó build:

```bash
cmake --build build
```

Nếu build thành công, file chạy sẽ nằm trong thư mục:

build/PhongLighting.exe

## 4. Chạy chương trình

Trong terminal MSYS2 UCRT64, chạy:

```bash
./build/PhongLighting.exe
```

Nếu chương trình chạy đúng, một cửa sổ OpenGL sẽ xuất hiện. Trong cửa sổ đó, vật thể 3D sẽ được chiếu sáng bằng mô hình Phong hoặc Blinn-Phong


## 5 Controls

- ESC: Exit
- Arrow keys: Move light on X/Y axes
- Z / X: Move light on Z axis
- Q / E: Decrease / increase shininess
- P: Phong mode
- B: Blinn-Phong mode
- W / S: Move camera forward / backward
