#include "UI.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>

namespace {
const glm::vec4 PANEL_BG(0.025f, 0.055f, 0.080f, 0.96f);
const glm::vec4 PANEL_LINE(0.0f, 0.86f, 0.92f, 1.0f);
const glm::vec4 TEXT(0.90f, 0.96f, 0.98f, 1.0f);
const glm::vec4 MUTED(0.58f, 0.68f, 0.72f, 1.0f);
const glm::vec4 ACCENT(0.0f, 0.82f, 0.88f, 1.0f);
const glm::vec4 CONTROL(0.075f, 0.120f, 0.145f, 1.0f);

using GlyphRows = std::array<unsigned char, 7>;

GlyphRows glyphRows(char c) {
    switch (static_cast<char>(std::toupper(static_cast<unsigned char>(c)))) {
        case 'A': return {14, 17, 17, 31, 17, 17, 17};
        case 'B': return {30, 17, 17, 30, 17, 17, 30};
        case 'C': return {14, 17, 16, 16, 16, 17, 14};
        case 'D': return {30, 17, 17, 17, 17, 17, 30};
        case 'E': return {31, 16, 16, 30, 16, 16, 31};
        case 'F': return {31, 16, 16, 30, 16, 16, 16};
        case 'G': return {14, 17, 16, 23, 17, 17, 14};
        case 'H': return {17, 17, 17, 31, 17, 17, 17};
        case 'I': return {14, 4, 4, 4, 4, 4, 14};
        case 'J': return {7, 2, 2, 2, 18, 18, 12};
        case 'K': return {17, 18, 20, 24, 20, 18, 17};
        case 'L': return {16, 16, 16, 16, 16, 16, 31};
        case 'M': return {17, 27, 21, 21, 17, 17, 17};
        case 'N': return {17, 25, 21, 19, 17, 17, 17};
        case 'O': return {14, 17, 17, 17, 17, 17, 14};
        case 'P': return {30, 17, 17, 30, 16, 16, 16};
        case 'Q': return {14, 17, 17, 17, 21, 18, 13};
        case 'R': return {30, 17, 17, 30, 20, 18, 17};
        case 'S': return {15, 16, 16, 14, 1, 1, 30};
        case 'T': return {31, 4, 4, 4, 4, 4, 4};
        case 'U': return {17, 17, 17, 17, 17, 17, 14};
        case 'V': return {17, 17, 17, 17, 17, 10, 4};
        case 'W': return {17, 17, 17, 21, 21, 21, 10};
        case 'X': return {17, 17, 10, 4, 10, 17, 17};
        case 'Y': return {17, 17, 10, 4, 4, 4, 4};
        case 'Z': return {31, 1, 2, 4, 8, 16, 31};
        case '0': return {14, 17, 19, 21, 25, 17, 14};
        case '1': return {4, 12, 4, 4, 4, 4, 14};
        case '2': return {14, 17, 1, 2, 4, 8, 31};
        case '3': return {30, 1, 1, 14, 1, 1, 30};
        case '4': return {2, 6, 10, 18, 31, 2, 2};
        case '5': return {31, 16, 16, 30, 1, 1, 30};
        case '6': return {14, 16, 16, 30, 17, 17, 14};
        case '7': return {31, 1, 2, 4, 8, 8, 8};
        case '8': return {14, 17, 17, 14, 17, 17, 14};
        case '9': return {14, 17, 17, 15, 1, 1, 14};
        case '.': return {0, 0, 0, 0, 0, 12, 12};
        case ':': return {0, 12, 12, 0, 12, 12, 0};
        case '-': return {0, 0, 0, 14, 0, 0, 0};
        case '+': return {0, 4, 4, 31, 4, 4, 0};
        case '/': return {1, 1, 2, 4, 8, 16, 16};
        case ' ': return {0, 0, 0, 0, 0, 0, 0};
        default: return {0, 0, 14, 4, 4, 0, 4};
    }
}
}

UI::UI(const char* vertexPath, const char* fragmentPath) : shader(vertexPath, fragmentPath) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

UI::~UI() {
}

void UI::begin(GLFWwindow* window, int w, int h, int panelW) {
    width = w;
    height = h;
    panelWidth = panelW;
    double cursorX = 0.0;
    double cursorY = 0.0;
    int winW = width;
    int winH = height;
    glfwGetCursorPos(window, &cursorX, &cursorY);
    glfwGetWindowSize(window, &winW, &winH);
    mouseX = cursorX * static_cast<double>(width) / static_cast<double>(std::max(1, winW));
    mouseY = cursorY * static_cast<double>(height) / static_cast<double>(std::max(1, winH));
    mouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    mousePressed = mouseDown && !lastMouseDown;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader.use();
    shader.setMat4("projection", glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f));
}

void UI::end() {
    lastMouseDown = mouseDown;
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void UI::panel() {
    rect(static_cast<float>(width - panelWidth), 0.0f, static_cast<float>(panelWidth), static_cast<float>(height), PANEL_BG);
    rect(static_cast<float>(width - panelWidth), 0.0f, 4.0f, static_cast<float>(height), PANEL_LINE);
    rect(static_cast<float>(width - panelWidth + 10), 12.0f, static_cast<float>(panelWidth - 20), static_cast<float>(height - 24), glm::vec4(0.02f, 0.045f, 0.065f, 0.42f));
}

void UI::separator(float x, float y, float w) {
    rect(x, y, w, 1.0f, glm::vec4(0.12f, 0.19f, 0.21f, 1.0f));
}

void UI::text(float x, float y, const std::string& value, const glm::vec4& color, float scale) {
    float cursor = x;
    for (char c : value) {
        glyph(cursor, y, c, color, scale);
        cursor += 6.0f * scale;
    }
}

bool UI::button(float x, float y, float w, float h, const std::string& label, bool active) {
    bool inside = hit(x, y, w, h);
    glm::vec4 color = active ? ACCENT : (inside ? glm::vec4(0.12f, 0.20f, 0.23f, 1.0f) : CONTROL);
    rect(x, y, w, h, color);
    if (!active) {
        rect(x, y, w, 1.0f, glm::vec4(0.18f, 0.27f, 0.30f, 1.0f));
    }
    float scale = 2.15f;
    float textW = static_cast<float>(label.size()) * 6.0f * scale;
    text(x + (w - textW) * 0.5f, y + (h - 7.0f * scale) * 0.5f, label,
         active ? glm::vec4(0.01f, 0.04f, 0.05f, 1.0f) : TEXT, scale);
    return inside && mousePressed;
}

bool UI::checkbox(float x, float y, const std::string& label, bool checked) {
    bool inside = hit(x, y, 320.0f, 28.0f);
    rect(x, y + 2.0f, 20.0f, 20.0f, checked ? ACCENT : glm::vec4(0.28f, 0.34f, 0.36f, 1.0f));
    if (checked) {
        rect(x + 5.0f, y + 7.0f, 10.0f, 10.0f, glm::vec4(0.01f, 0.04f, 0.05f, 1.0f));
    } else {
        rect(x + 3.0f, y + 5.0f, 14.0f, 14.0f, glm::vec4(0.025f, 0.055f, 0.080f, 1.0f));
    }
    text(x + 34.0f, y + 1.0f, label, inside ? TEXT : MUTED, 2.05f);
    return inside && mousePressed;
}

bool UI::slider(float x, float y, float w, const std::string& label, float& value, float minValue, float maxValue) {
    std::ostringstream valueText;
    valueText.precision(2);
    valueText << std::fixed << value;
    text(x, y, label, MUTED, 1.72f);
    text(x + w - 58.0f, y, valueText.str(), TEXT, 1.62f);
    float trackY = y + 29.0f;
    rect(x, trackY, w, 5.0f, CONTROL);
    float t = (value - minValue) / (maxValue - minValue);
    t = std::clamp(t, 0.0f, 1.0f);
    rect(x, trackY, w * t, 5.0f, ACCENT);
    rect(x + w * t - 8.0f, trackY - 7.0f, 16.0f, 19.0f, TEXT);

    bool inside = hit(x, trackY - 10.0f, w, 26.0f);
    if (inside && mouseDown) {
        float newT = std::clamp((static_cast<float>(mouseX) - x) / w, 0.0f, 1.0f);
        value = minValue + (maxValue - minValue) * newT;
        return true;
    }
    return false;
}

bool UI::hit(float x, float y, float w, float h) const {
    return mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h;
}

void UI::rect(float x, float y, float w, float h, const glm::vec4& color) {
    float vertices[] = {
        x, y, x + w, y, x + w, y + h,
        x, y, x + w, y + h, x, y + h
    };
    shader.use();
    shader.setVec4("color", color);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void UI::glyph(float x, float y, char c, const glm::vec4& color, float scale) {
    GlyphRows rows = glyphRows(c);
    for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 5; ++col) {
            if ((rows[row] >> (4 - col)) & 1) {
                rect(x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}
