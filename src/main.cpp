#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

const glm::vec3 DEFAULT_LIGHT_POS(1.2f, 2.5f, 2.0f);
const glm::vec3 DEFAULT_CAMERA_POS(0.0f, 1.0f, 6.0f);

glm::vec3 lightPos = DEFAULT_LIGHT_POS;
glm::vec3 cameraPos = DEFAULT_CAMERA_POS;

float shininess = 32.0f;
bool useBlinn = false;
int lightingMode = 4;
bool enableShadow = true;
bool enableAttenuation = true;
bool autoOrbitLight = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool keyPressed[GLFW_KEY_LAST + 1] = {};

unsigned int cubeVAO = 0;
unsigned int lightVAO = 0;
unsigned int planeVAO = 0;
unsigned int sphereVAO = 0;
GLsizei sphereIndexCount = 0;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
bool isPressedOnce(GLFWwindow* window, int key);
void resetScene();
void printControls();
void drawScene(const Shader& shader, bool depthOnly);
void generateSphere(float radius,
                    int sectorCount,
                    int stackCount,
                    std::vector<float>& vertices,
                    std::vector<unsigned int>& indices);
std::string lightingModeName();
void updateWindowTitle(GLFWwindow* window);

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        SCR_WIDTH,
        SCR_HEIGHT,
        "Phong / Blinn-Phong Lighting - OpenGL",
        nullptr,
        nullptr
    );

    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: "
                  << glewGetErrorString(glewStatus) << "\n";
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader lightingShader("shaders/phong.vert", "shaders/phong.frag");
    Shader lampShader("shaders/lamp.vert", "shaders/lamp.frag");
    Shader depthShader("shaders/depth.vert", "shaders/depth.frag");

    float cubeVertices[] = {
        // positions             // normals
        -0.5f,-0.5f,-0.5f,       0.0f, 0.0f,-1.0f,
         0.5f,-0.5f,-0.5f,       0.0f, 0.0f,-1.0f,
         0.5f, 0.5f,-0.5f,       0.0f, 0.0f,-1.0f,
         0.5f, 0.5f,-0.5f,       0.0f, 0.0f,-1.0f,
        -0.5f, 0.5f,-0.5f,       0.0f, 0.0f,-1.0f,
        -0.5f,-0.5f,-0.5f,       0.0f, 0.0f,-1.0f,

        -0.5f,-0.5f, 0.5f,       0.0f, 0.0f, 1.0f,
         0.5f,-0.5f, 0.5f,       0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 0.5f,       0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 0.5f,       0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f,       0.0f, 0.0f, 1.0f,
        -0.5f,-0.5f, 0.5f,       0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, 0.5f,      -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f,-0.5f,      -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f,-0.5f,      -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f,-0.5f,      -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f, 0.5f,      -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,      -1.0f, 0.0f, 0.0f,

         0.5f, 0.5f, 0.5f,       1.0f, 0.0f, 0.0f,
         0.5f, 0.5f,-0.5f,       1.0f, 0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,       1.0f, 0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,       1.0f, 0.0f, 0.0f,
         0.5f,-0.5f, 0.5f,       1.0f, 0.0f, 0.0f,
         0.5f, 0.5f, 0.5f,       1.0f, 0.0f, 0.0f,

        -0.5f,-0.5f,-0.5f,       0.0f,-1.0f, 0.0f,
         0.5f,-0.5f,-0.5f,       0.0f,-1.0f, 0.0f,
         0.5f,-0.5f, 0.5f,       0.0f,-1.0f, 0.0f,
         0.5f,-0.5f, 0.5f,       0.0f,-1.0f, 0.0f,
        -0.5f,-0.5f, 0.5f,       0.0f,-1.0f, 0.0f,
        -0.5f,-0.5f,-0.5f,       0.0f,-1.0f, 0.0f,

        -0.5f, 0.5f,-0.5f,       0.0f, 1.0f, 0.0f,
         0.5f, 0.5f,-0.5f,       0.0f, 1.0f, 0.0f,
         0.5f, 0.5f, 0.5f,       0.0f, 1.0f, 0.0f,
         0.5f, 0.5f, 0.5f,       0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,       0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f,-0.5f,       0.0f, 1.0f, 0.0f
    };

    float planeVertices[] = {
        // positions             // normals
        -8.0f, -1.0f, -8.0f,     0.0f, 1.0f, 0.0f,
         8.0f, -1.0f, -8.0f,     0.0f, 1.0f, 0.0f,
         8.0f, -1.0f,  8.0f,     0.0f, 1.0f, 0.0f,
         8.0f, -1.0f,  8.0f,     0.0f, 1.0f, 0.0f,
        -8.0f, -1.0f,  8.0f,     0.0f, 1.0f, 0.0f,
        -8.0f, -1.0f, -8.0f,     0.0f, 1.0f, 0.0f
    };

    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    generateSphere(0.65f, 48, 24, sphereVertices, sphereIndices);
    sphereIndexCount = static_cast<GLsizei>(sphereIndices.size());

    unsigned int cubeVBO = 0;
    unsigned int planeVBO = 0;
    unsigned int sphereVBO = 0;
    unsigned int sphereEBO = 0;

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(sphereVertices.size() * sizeof(float)),
                 sphereVertices.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(sphereIndices.size() * sizeof(unsigned int)),
                 sphereIndices.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    unsigned int depthMapFBO = 0;
    unsigned int depthMap = 0;
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER:: Shadow framebuffer is not complete\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    lightingShader.use();
    lightingShader.setInt("shadowMap", 0);

    printControls();

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        if (autoOrbitLight) {
            const float radius = 3.0f;
            lightPos.x = std::cos(currentFrame) * radius;
            lightPos.y = 3.0f;
            lightPos.z = std::sin(currentFrame) * radius;
        }

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
            0.1f,
            100.0f
        );

        glm::mat4 view = glm::lookAt(
            cameraPos,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glm::mat4 lightProjection = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, 1.0f, 20.0f);
        glm::mat4 lightView = glm::lookAt(lightPos,
                                          glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        drawScene(depthShader, true);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightingShader.use();
        lightingShader.setVec3("lightColor", glm::vec3(1.0f));
        lightingShader.setVec3("lightPos", lightPos);
        lightingShader.setVec3("viewPos", cameraPos);
        lightingShader.setFloat("shininess", shininess);
        lightingShader.setBool("useBlinn", useBlinn);
        lightingShader.setInt("lightingMode", lightingMode);
        lightingShader.setBool("enableShadow", enableShadow);
        lightingShader.setBool("enableAttenuation", enableAttenuation);
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        drawScene(lightingShader, false);

        lampShader.use();
        lampShader.setVec3("lightColor", glm::vec3(1.0f));
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", view);

        glm::mat4 lampModel = glm::mat4(1.0f);
        lampModel = glm::translate(lampModel, lightPos);
        lampModel = glm::scale(lampModel, glm::vec3(0.2f));
        lampShader.setMat4("model", lampModel);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        updateWindowTitle(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthMap);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    const float lightSpeed = 2.5f * deltaTime;
    const float cameraSpeed = 3.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (!autoOrbitLight) {
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            lightPos.x -= lightSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            lightPos.x += lightSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            lightPos.y += lightSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            lightPos.y -= lightSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            lightPos.z -= lightSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            lightPos.z += lightSpeed;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        shininess = glm::max(2.0f, shininess - 40.0f * deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        shininess = glm::min(256.0f, shininess + 40.0f * deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos.z = glm::max(2.0f, cameraPos.z - cameraSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos.z = glm::min(12.0f, cameraPos.z + cameraSpeed);
    }

    if (isPressedOnce(window, GLFW_KEY_P)) {
        useBlinn = false;
    }
    if (isPressedOnce(window, GLFW_KEY_B)) {
        useBlinn = true;
    }
    if (isPressedOnce(window, GLFW_KEY_A)) {
        enableAttenuation = !enableAttenuation;
    }
    if (isPressedOnce(window, GLFW_KEY_O)) {
        enableShadow = !enableShadow;
    }
    if (isPressedOnce(window, GLFW_KEY_T)) {
        autoOrbitLight = !autoOrbitLight;
    }
    if (isPressedOnce(window, GLFW_KEY_R)) {
        resetScene();
    }
    if (isPressedOnce(window, GLFW_KEY_1)) {
        lightingMode = 1;
    }
    if (isPressedOnce(window, GLFW_KEY_2)) {
        lightingMode = 2;
    }
    if (isPressedOnce(window, GLFW_KEY_3)) {
        lightingMode = 3;
    }
    if (isPressedOnce(window, GLFW_KEY_4)) {
        lightingMode = 4;
    }
}

bool isPressedOnce(GLFWwindow* window, int key) {
    bool isPressed = glfwGetKey(window, key) == GLFW_PRESS;
    bool fire = isPressed && !keyPressed[key];
    keyPressed[key] = isPressed;
    return fire;
}

void resetScene() {
    lightPos = DEFAULT_LIGHT_POS;
    cameraPos = DEFAULT_CAMERA_POS;
    shininess = 32.0f;
    useBlinn = false;
    lightingMode = 4;
    enableShadow = true;
    enableAttenuation = true;
    autoOrbitLight = false;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

void printControls() {
    std::cout << "Controls:\n"
              << "ESC: Exit\n"
              << "Arrow keys: Move light on X/Y axes\n"
              << "Z / X: Move light on Z axis\n"
              << "W / S: Move camera forward / backward\n"
              << "Q / E: Decrease / increase shininess\n"
              << "P: Phong mode\n"
              << "B: Blinn-Phong mode\n"
              << "1: Ambient only\n"
              << "2: Diffuse only\n"
              << "3: Specular only\n"
              << "4: Full lighting\n"
              << "O: Toggle Shadow Mapping\n"
              << "A: Toggle attenuation\n"
              << "T: Toggle auto-orbit light\n"
              << "R: Reset scene\n";
}

void drawScene(const Shader& shader, bool depthOnly) {
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    if (!depthOnly) {
        shader.setVec3("objectColor", glm::vec3(0.55f, 0.55f, 0.58f));
    }
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.25f, -0.35f, 0.0f));
    model = glm::rotate(model, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    shader.setMat4("model", model);
    if (!depthOnly) {
        shader.setVec3("objectColor", glm::vec3(1.0f, 0.42f, 0.22f));
    }
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.35f, -0.35f, 0.2f));
    shader.setMat4("model", model);
    if (!depthOnly) {
        shader.setVec3("objectColor", glm::vec3(0.0f, 0.72f, 0.68f));
    }
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, nullptr);
}

void generateSphere(float radius,
                    int sectorCount,
                    int stackCount,
                    std::vector<float>& vertices,
                    std::vector<unsigned int>& indices) {
    const float PI = 3.14159265359f;

    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = PI / 2.0f - static_cast<float>(i) * PI / static_cast<float>(stackCount);
        float xy = radius * std::cos(stackAngle);
        float z = radius * std::sin(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = static_cast<float>(j) * 2.0f * PI / static_cast<float>(sectorCount);
            float x = xy * std::cos(sectorAngle);
            float y = xy * std::sin(sectorAngle);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(x / radius);
            vertices.push_back(y / radius);
            vertices.push_back(z / radius);
        }
    }

    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(static_cast<unsigned int>(k1));
                indices.push_back(static_cast<unsigned int>(k2));
                indices.push_back(static_cast<unsigned int>(k1 + 1));
            }

            if (i != stackCount - 1) {
                indices.push_back(static_cast<unsigned int>(k1 + 1));
                indices.push_back(static_cast<unsigned int>(k2));
                indices.push_back(static_cast<unsigned int>(k2 + 1));
            }
        }
    }
}

std::string lightingModeName() {
    switch (lightingMode) {
        case 1:
            return "Ambient only";
        case 2:
            return "Diffuse only";
        case 3:
            return "Specular only";
        default:
            return "Full";
    }
}

void updateWindowTitle(GLFWwindow* window) {
    std::ostringstream title;
    title << (useBlinn ? "Blinn-Phong" : "Phong")
          << " | Shininess: " << static_cast<int>(shininess)
          << " | Lighting: " << lightingModeName()
          << " | Shadow: " << (enableShadow ? "ON" : "OFF")
          << " | Attenuation: " << (enableAttenuation ? "ON" : "OFF")
          << " | Orbit: " << (autoOrbitLight ? "ON" : "OFF");
    glfwSetWindowTitle(window, title.str().c_str());
}
