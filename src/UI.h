#ifndef UI_H
#define UI_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Shader.h"

#include <string>

class UI {
public:
    explicit UI(const char* vertexPath, const char* fragmentPath);
    ~UI();

    void begin(GLFWwindow* window, int width, int height, int panelWidth);
    void end();

    void panel();
    void separator(float x, float y, float w);
    void text(float x, float y, const std::string& value, const glm::vec4& color, float scale = 2.0f);
    bool button(float x, float y, float w, float h, const std::string& label, bool active = false);
    bool checkbox(float x, float y, const std::string& label, bool checked);
    bool slider(float x, float y, float w, const std::string& label, float& value, float minValue, float maxValue);

private:
    Shader shader;
    unsigned int vao = 0;
    unsigned int vbo = 0;
    int width = 0;
    int height = 0;
    int panelWidth = 0;
    double mouseX = 0.0;
    double mouseY = 0.0;
    bool mouseDown = false;
    bool mousePressed = false;
    bool lastMouseDown = false;

    bool hit(float x, float y, float w, float h) const;
    void rect(float x, float y, float w, float h, const glm::vec4& color);
    void glyph(float x, float y, char c, const glm::vec4& color, float scale);
};

#endif
