#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

#include <iostream>
#include <string>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);

float shininess = 32.0f;
bool useBlinn = false;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void printControls();

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

    float vertices[] = {
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

    unsigned int VBO, cubeVAO, lightVAO;

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        reinterpret_cast<void*>(0)
    );
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        reinterpret_cast<void*>(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        reinterpret_cast<void*>(0)
    );
    glEnableVertexAttribArray(0);

    printControls();

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

        lightingShader.use();
        lightingShader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
        lightingShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        lightingShader.setVec3("lightPos", lightPos);
        lightingShader.setVec3("viewPos", cameraPos);
        lightingShader.setFloat("shininess", shininess);
        lightingShader.setBool("useBlinn", useBlinn);
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, static_cast<float>(glfwGetTime()) * 0.3f,
                            glm::vec3(0.3f, 1.0f, 0.0f));
        lightingShader.setMat4("model", model);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

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

        std::string title = std::string("Phong / Blinn-Phong Lighting - Mode: ")
                          + (useBlinn ? "Blinn-Phong" : "Phong")
                          + " | Shininess: " + std::to_string(static_cast<int>(shininess));
        glfwSetWindowTitle(window, title.c_str());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    const float lightSpeed = 0.008f;
    const float cameraSpeed = 0.05f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Move light
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

    // Adjust specular highlight
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        shininess = glm::max(2.0f, shininess - 0.2f);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        shininess = glm::min(256.0f, shininess + 0.2f);
    }

    // Switch lighting model
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        useBlinn = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        useBlinn = false;
    }

    // Move camera forward/backward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos.z -= cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos.z += cameraSpeed;
    }
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
              << "Q / E: Decrease / increase shininess\n"
              << "P: Phong mode\n"
              << "B: Blinn-Phong mode\n"
              << "W / S: Move camera forward / backward\n";
}
