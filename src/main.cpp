#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "UI.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;
const int PANEL_MIN_WIDTH = 420;
const int PANEL_MAX_WIDTH = 560;
const float ROOM_WIDTH = 12.0f;
const float ROOM_DEPTH = 10.0f;
const float ROOM_HEIGHT = 6.0f;
const float SIDE_WALL_DEPTH = 6.2f;
const float FLOOR_Y = -1.0f;

const glm::vec3 DEFAULT_LIGHT_POS(1.5f, 3.0f, 2.0f);
const glm::vec3 DEFAULT_CAMERA_POS(0.0f, 1.0f, 6.0f);

struct MaterialPreset
{
    std::string name;
    glm::vec3 cubeColor;
    glm::vec3 sphereColor;
    glm::vec3 floorColor;
    float shininess;
    float specularStrength;
    float ambientStrength;
    float metallic;
    float roughness;
    float ao;
    float materialFactor;
};

enum class ObjectType
{
    Cube,
    Sphere,
    Pyramid
};

struct SceneObject
{
    ObjectType type;
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 color;
    float rotationY;
    bool visible;
    std::string name;
};

const std::array<MaterialPreset, 4> MATERIAL_PRESETS = {{{"Plastic", glm::vec3(1.0f, 0.42f, 0.22f), glm::vec3(0.0f, 0.72f, 0.68f), glm::vec3(0.55f, 0.55f, 0.58f), 32.0f, 0.50f, 0.12f, 0.0f, 0.45f, 1.0f, 0.45f},
                                                         {"Rubber", glm::vec3(0.42f, 0.16f, 0.10f), glm::vec3(0.02f, 0.28f, 0.26f), glm::vec3(0.28f, 0.28f, 0.30f), 8.0f, 0.18f, 0.14f, 0.0f, 0.85f, 1.0f, 0.25f},
                                                         {"Metal-like", glm::vec3(0.70f, 0.66f, 0.58f), glm::vec3(0.55f, 0.62f, 0.66f), glm::vec3(0.48f, 0.48f, 0.50f), 128.0f, 0.90f, 0.08f, 0.8f, 0.25f, 1.0f, 0.80f},
                                                         {"Ceramic", glm::vec3(0.92f, 0.74f, 0.58f), glm::vec3(0.72f, 0.92f, 0.88f), glm::vec3(0.68f, 0.68f, 0.70f), 64.0f, 0.55f, 0.13f, 0.0f, 0.30f, 1.0f, 0.55f}}};

glm::vec3 lightPos = DEFAULT_LIGHT_POS;
glm::vec3 cameraPos = DEFAULT_CAMERA_POS;

float shininess = 32.0f;
float metallic = 0.0f;
float roughness = 0.45f;
float ao = 1.0f;
float shadowStrength = 0.65f;
int renderMode = 0;
int lightingMode = 4;
bool enableAmbient = true;
bool enableDiffuse = true;
bool enableSpecular = true;
bool enableShadow = true;
bool enableAttenuation = true;
bool autoOrbitLight = false;
bool enableGI = false;
bool enableAIGI = false;
float giStrength = 1.0f;
float colorBleedingStrength = 0.8f;
float bounceStrength = 0.8f;
float aiStrength = 0.35f;
float lightIntensity = 4.0f;
float uiScrollOffset = 0.0f;
std::size_t currentMaterialIndex = 0;
bool multiObjectMode = true;
ObjectType selectedObjectType = ObjectType::Cube;
int selectedObjectIndex = 0;
std::vector<SceneObject> sceneObjects;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool keyPressed[GLFW_KEY_LAST + 1] = {};
bool isFullscreen = false;
int windowedX = 100;
int windowedY = 100;
int windowedWidth = SCR_WIDTH;
int windowedHeight = SCR_HEIGHT;
int framebufferWidth = SCR_WIDTH;
int framebufferHeight = SCR_HEIGHT;
int windowWidth = SCR_WIDTH;
int windowHeight = SCR_HEIGHT;
int panelWidth = PANEL_MIN_WIDTH;
int sceneWidth = SCR_WIDTH - PANEL_MIN_WIDTH;

unsigned int cubeVAO = 0;
unsigned int lightVAO = 0;
unsigned int planeVAO = 0;
unsigned int sphereVAO = 0;
unsigned int pyramidVAO = 0;
GLsizei sphereIndexCount = 0;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
bool isPressedOnce(GLFWwindow *window, int key);
void resetScene();
void applyMaterialPreset(std::size_t index);
void toggleFullscreen(GLFWwindow *window);
void updateLayout(GLFWwindow *window);
void printControls();
void drawScene(const Shader &shader, bool depthOnly);
void drawObject(const SceneObject &object, const Shader &shader, bool depthOnly);
void drawControlPanel(UI &ui, GLFWwindow *window);
void initializeDefaultObjects();
SceneObject makeSceneObject(ObjectType type, int index);
void addSceneObject(ObjectType type);
void removeSelectedObject();
void clearSceneObjects();
void setSingleObjectType(ObjectType type);
void randomizeSelectedObjectColor();
glm::vec3 randomColor();
glm::vec3 getDefaultPositionForObject(int index, ObjectType type);
glm::vec3 getDefaultScaleForObject(ObjectType type);
glm::vec3 getDefaultColorForObject(ObjectType type);
std::string objectTypeName(ObjectType type);
ObjectType nextObjectType(ObjectType type);
void generateSphere(float radius,
                    int sectorCount,
                    int stackCount,
                    std::vector<float> &vertices,
                    std::vector<unsigned int> &indices);
std::string lightingModeName();
std::string renderModeName();
void updateWindowTitle(GLFWwindow *window);

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *desktopMode = glfwGetVideoMode(primaryMonitor);
    int initialWindowWidth = static_cast<int>(SCR_WIDTH);
    int initialWindowHeight = static_cast<int>(SCR_HEIGHT);
    if (desktopMode != nullptr &&
        (desktopMode->width < static_cast<int>(SCR_WIDTH) || desktopMode->height < static_cast<int>(SCR_HEIGHT)))
    {
        initialWindowWidth = static_cast<int>(desktopMode->width * 0.9f);
        initialWindowHeight = static_cast<int>(desktopMode->height * 0.9f);
    }
    windowedWidth = initialWindowWidth;
    windowedHeight = initialWindowHeight;

    GLFWwindow *window = glfwCreateWindow(
        initialWindowWidth,
        initialWindowHeight,
        "Phong / Blinn-Phong Lighting - OpenGL",
        nullptr,
        nullptr);

    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    updateLayout(window);

    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW: "
                  << glewGetErrorString(glewStatus) << "\n";
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader lightingShader("shaders/phong.vert", "shaders/phong.frag");
    Shader lampShader("shaders/lamp.vert", "shaders/lamp.frag");
    Shader depthShader("shaders/depth.vert", "shaders/depth.frag");
    UI ui("shaders/ui.vert", "shaders/ui.frag");

    float cubeVertices[] = {
        // positions             // normals
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f};

    float planeVertices[] = {
        // positions             // normals
        -ROOM_WIDTH * 0.5f, FLOOR_Y, -ROOM_DEPTH * 0.5f, 0.0f, 1.0f, 0.0f,
        ROOM_WIDTH * 0.5f, FLOOR_Y, -ROOM_DEPTH * 0.5f, 0.0f, 1.0f, 0.0f,
        ROOM_WIDTH * 0.5f, FLOOR_Y, ROOM_DEPTH * 0.5f, 0.0f, 1.0f, 0.0f,
        ROOM_WIDTH * 0.5f, FLOOR_Y, ROOM_DEPTH * 0.5f, 0.0f, 1.0f, 0.0f,
        -ROOM_WIDTH * 0.5f, FLOOR_Y, ROOM_DEPTH * 0.5f, 0.0f, 1.0f, 0.0f,
        -ROOM_WIDTH * 0.5f, FLOOR_Y, -ROOM_DEPTH * 0.5f, 0.0f, 1.0f, 0.0f};

    float pyramidVertices[] = {
        // front face
        -0.5f, 0.0f, 0.5f, 0.0f, 0.447f, 0.894f,
         0.5f, 0.0f, 0.5f, 0.0f, 0.447f, 0.894f,
         0.0f, 1.0f, 0.0f, 0.0f, 0.447f, 0.894f,
        // right face
         0.5f, 0.0f, 0.5f, 0.894f, 0.447f, 0.0f,
         0.5f, 0.0f,-0.5f, 0.894f, 0.447f, 0.0f,
         0.0f, 1.0f, 0.0f, 0.894f, 0.447f, 0.0f,
        // back face
         0.5f, 0.0f,-0.5f, 0.0f, 0.447f,-0.894f,
        -0.5f, 0.0f,-0.5f, 0.0f, 0.447f,-0.894f,
         0.0f, 1.0f, 0.0f, 0.0f, 0.447f,-0.894f,
        // left face
        -0.5f, 0.0f,-0.5f,-0.894f, 0.447f, 0.0f,
        -0.5f, 0.0f, 0.5f,-0.894f, 0.447f, 0.0f,
         0.0f, 1.0f, 0.0f,-0.894f, 0.447f, 0.0f,
        // base
        -0.5f, 0.0f,-0.5f, 0.0f,-1.0f, 0.0f,
         0.5f, 0.0f,-0.5f, 0.0f,-1.0f, 0.0f,
         0.5f, 0.0f, 0.5f, 0.0f,-1.0f, 0.0f,
         0.5f, 0.0f, 0.5f, 0.0f,-1.0f, 0.0f,
        -0.5f, 0.0f, 0.5f, 0.0f,-1.0f, 0.0f,
        -0.5f, 0.0f,-0.5f, 0.0f,-1.0f, 0.0f
    };

    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    generateSphere(0.65f, 48, 24, sphereVertices, sphereIndices);
    sphereIndexCount = static_cast<GLsizei>(sphereIndices.size());

    unsigned int cubeVBO = 0;
    unsigned int planeVBO = 0;
    unsigned int sphereVBO = 0;
    unsigned int sphereEBO = 0;
    unsigned int pyramidVBO = 0;

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(0);

    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);
    glBindVertexArray(pyramidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));
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
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR::FRAMEBUFFER:: Shadow framebuffer is not complete\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    lightingShader.use();
    lightingShader.setInt("shadowMap", 0);
    initializeDefaultObjects();

    printControls();

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        updateLayout(window);
        processInput(window);

        if (autoOrbitLight)
        {
            const float radius = 3.0f;
            lightPos.x = std::cos(currentFrame) * radius;
            lightPos.y = 3.0f;
            lightPos.z = std::sin(currentFrame) * radius;
        }

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(sceneWidth) / static_cast<float>(framebufferHeight),
            0.1f,
            100.0f);

        glm::mat4 view = glm::lookAt(
            cameraPos,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));

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

        glViewport(0, 0, framebufferWidth, framebufferHeight);
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, sceneWidth, framebufferHeight);

        lightingShader.use();
        lightingShader.setVec3("lightColor", glm::vec3(1.0f));
        lightingShader.setVec3("lightPos", lightPos);
        lightingShader.setVec3("viewPos", cameraPos);
        lightingShader.setInt("renderMode", renderMode);
        lightingShader.setFloat("shininess", shininess);
        lightingShader.setFloat("metallic", metallic);
        lightingShader.setFloat("roughness", roughness);
        lightingShader.setFloat("ao", ao);
        lightingShader.setFloat("lightIntensity", lightIntensity);
        lightingShader.setFloat("shadowStrength", shadowStrength);
        lightingShader.setFloat("specularStrength", MATERIAL_PRESETS[currentMaterialIndex].specularStrength);
        lightingShader.setFloat("ambientStrength", MATERIAL_PRESETS[currentMaterialIndex].ambientStrength);
        lightingShader.setFloat("giStrength", giStrength);
        lightingShader.setFloat("colorBleedingStrength", colorBleedingStrength);
        lightingShader.setFloat("bounceStrength", bounceStrength);
        lightingShader.setFloat("aiStrength", aiStrength);
        lightingShader.setFloat("materialFactor", MATERIAL_PRESETS[currentMaterialIndex].materialFactor);
        lightingShader.setBool("enableAmbient", enableAmbient);
        lightingShader.setBool("enableDiffuse", enableDiffuse);
        lightingShader.setBool("enableSpecular", enableSpecular);
        lightingShader.setBool("enableGI", enableGI);
        lightingShader.setBool("enableAIGI", enableAIGI);
        lightingShader.setInt("lightingMode", lightingMode);
        lightingShader.setBool("enableShadow", enableShadow);
        lightingShader.setBool("enableAttenuation", enableAttenuation);
        lightingShader.setVec3("floorColor", MATERIAL_PRESETS[currentMaterialIndex].floorColor);
        lightingShader.setVec3("leftWallColor", glm::vec3(0.75f, 0.12f, 0.10f));
        lightingShader.setVec3("rightWallColor", glm::vec3(0.10f, 0.22f, 0.85f));
        lightingShader.setVec3("backWallColor", glm::vec3(0.22f, 0.24f, 0.28f));
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        drawScene(lightingShader, false);

        lampShader.use();
        lampShader.setVec3("lightColor", glm::vec3(1.0f, 0.92f, 0.60f));
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", view);

        glm::mat4 lampModel = glm::mat4(1.0f);
        lampModel = glm::translate(lampModel, lightPos);
        lampModel = glm::scale(lampModel, glm::vec3(0.22f));
        lampShader.setMat4("model", lampModel);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glViewport(0, 0, framebufferWidth, framebufferHeight);
        ui.begin(window, framebufferWidth, framebufferHeight, panelWidth);
        drawControlPanel(ui, window);
        ui.end();

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
    glDeleteVertexArrays(1, &pyramidVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteBuffers(1, &pyramidVBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    const float lightSpeed = 2.5f * deltaTime;
    const float cameraSpeed = 3.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (!autoOrbitLight)
    {
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            lightPos.x -= lightSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            lightPos.x += lightSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            lightPos.y += lightSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            lightPos.y -= lightSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        {
            lightPos.z -= lightSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        {
            lightPos.z += lightSpeed;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        shininess = glm::max(2.0f, shininess - 40.0f * deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        shininess = glm::min(256.0f, shininess + 40.0f * deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        shadowStrength = glm::max(0.0f, shadowStrength - 0.8f * deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        shadowStrength = glm::min(1.0f, shadowStrength + 0.8f * deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        aiStrength = glm::max(0.0f, aiStrength - 0.8f * deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        aiStrength = glm::min(1.0f, aiStrength + 0.8f * deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPos.z = glm::max(2.0f, cameraPos.z - cameraSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPos.z = glm::min(12.0f, cameraPos.z + cameraSpeed);
    }

    if (isPressedOnce(window, GLFW_KEY_P))
    {
        renderMode = 0;
    }
    if (isPressedOnce(window, GLFW_KEY_B))
    {
        renderMode = 1;
    }
    if (isPressedOnce(window, GLFW_KEY_V))
    {
        renderMode = 2;
    }
    if (isPressedOnce(window, GLFW_KEY_A))
    {
        enableAttenuation = !enableAttenuation;
    }
    if (isPressedOnce(window, GLFW_KEY_O))
    {
        enableShadow = !enableShadow;
    }
    if (isPressedOnce(window, GLFW_KEY_T))
    {
        autoOrbitLight = !autoOrbitLight;
    }
    if (isPressedOnce(window, GLFW_KEY_R))
    {
        resetScene();
    }
    if (isPressedOnce(window, GLFW_KEY_M))
    {
        applyMaterialPreset((currentMaterialIndex + 1) % MATERIAL_PRESETS.size());
    }
    if (isPressedOnce(window, GLFW_KEY_C))
    {
        setSingleObjectType(ObjectType::Cube);
    }
    if (isPressedOnce(window, GLFW_KEY_H))
    {
        setSingleObjectType(ObjectType::Sphere);
    }
    if (isPressedOnce(window, GLFW_KEY_Y))
    {
        setSingleObjectType(ObjectType::Pyramid);
    }
    if (isPressedOnce(window, GLFW_KEY_EQUAL) || isPressedOnce(window, GLFW_KEY_KP_ADD))
    {
        addSceneObject(selectedObjectType);
    }
    if (isPressedOnce(window, GLFW_KEY_DELETE))
    {
        removeSelectedObject();
    }
    if (isPressedOnce(window, GLFW_KEY_N))
    {
        randomizeSelectedObjectColor();
    }
    if (isPressedOnce(window, GLFW_KEY_G))
    {
        enableGI = !enableGI;
    }
    if (isPressedOnce(window, GLFW_KEY_I))
    {
        enableAIGI = !enableAIGI;
    }
    if (isPressedOnce(window, GLFW_KEY_F11))
    {
        toggleFullscreen(window);
    }
    if (isPressedOnce(window, GLFW_KEY_1))
    {
        lightingMode = 1;
        enableAmbient = true;
        enableDiffuse = false;
        enableSpecular = false;
    }
    if (isPressedOnce(window, GLFW_KEY_2))
    {
        lightingMode = 2;
        enableAmbient = false;
        enableDiffuse = true;
        enableSpecular = false;
    }
    if (isPressedOnce(window, GLFW_KEY_3))
    {
        lightingMode = 3;
        enableAmbient = false;
        enableDiffuse = false;
        enableSpecular = true;
    }
    if (isPressedOnce(window, GLFW_KEY_4))
    {
        lightingMode = 4;
        enableAmbient = true;
        enableDiffuse = true;
        enableSpecular = true;
    }
}

bool isPressedOnce(GLFWwindow *window, int key)
{
    bool isPressed = glfwGetKey(window, key) == GLFW_PRESS;
    bool fire = isPressed && !keyPressed[key];
    keyPressed[key] = isPressed;
    return fire;
}

void resetScene()
{
    lightPos = DEFAULT_LIGHT_POS;
    cameraPos = DEFAULT_CAMERA_POS;
    applyMaterialPreset(0);
    shadowStrength = 0.65f;
    renderMode = 0;
    lightingMode = 4;
    enableAmbient = true;
    enableDiffuse = true;
    enableSpecular = true;
    enableShadow = true;
    enableAttenuation = true;
    autoOrbitLight = false;
    enableGI = false;
    enableAIGI = false;
    giStrength = 1.0f;
    colorBleedingStrength = 0.8f;
    bounceStrength = 0.8f;
    aiStrength = 0.35f;
    lightIntensity = 4.0f;
    initializeDefaultObjects();
}

void applyMaterialPreset(std::size_t index)
{
    currentMaterialIndex = index;
    const MaterialPreset &material = MATERIAL_PRESETS[currentMaterialIndex];
    shininess = material.shininess;
    metallic = material.metallic;
    roughness = material.roughness;
    ao = material.ao;
}

void initializeDefaultObjects()
{
    multiObjectMode = true;
    sceneObjects.clear();
    sceneObjects.push_back(makeSceneObject(ObjectType::Cube, 0));
    sceneObjects.push_back(makeSceneObject(ObjectType::Sphere, 1));
    sceneObjects.push_back(makeSceneObject(ObjectType::Pyramid, 2));
    selectedObjectIndex = 0;
    selectedObjectType = ObjectType::Cube;
}

SceneObject makeSceneObject(ObjectType type, int index)
{
    SceneObject object;
    object.type = type;
    object.position = getDefaultPositionForObject(index, type);
    object.scale = getDefaultScaleForObject(type);
    object.color = getDefaultColorForObject(type);
    object.rotationY = type == ObjectType::Cube ? -15.0f : 0.0f;
    object.visible = true;
    object.name = objectTypeName(type);
    return object;
}

void addSceneObject(ObjectType type)
{
    if (!multiObjectMode)
    {
        multiObjectMode = true;
    }
    if (sceneObjects.size() >= 6)
    {
        return;
    }
    sceneObjects.push_back(makeSceneObject(type, static_cast<int>(sceneObjects.size())));
    selectedObjectIndex = static_cast<int>(sceneObjects.size()) - 1;
    selectedObjectType = type;
}

void removeSelectedObject()
{
    if (sceneObjects.empty())
    {
        initializeDefaultObjects();
        return;
    }
    if (!multiObjectMode && sceneObjects.size() <= 1)
    {
        return;
    }
    selectedObjectIndex = std::clamp(selectedObjectIndex, 0, static_cast<int>(sceneObjects.size()) - 1);
    sceneObjects.erase(sceneObjects.begin() + selectedObjectIndex);
    if (sceneObjects.empty())
    {
        sceneObjects.push_back(makeSceneObject(selectedObjectType, 0));
    }
    selectedObjectIndex = std::clamp(selectedObjectIndex, 0, static_cast<int>(sceneObjects.size()) - 1);
    selectedObjectType = sceneObjects[selectedObjectIndex].type;
}

void clearSceneObjects()
{
    initializeDefaultObjects();
}

void setSingleObjectType(ObjectType type)
{
    selectedObjectType = type;
    if (!multiObjectMode)
    {
        sceneObjects.clear();
        sceneObjects.push_back(makeSceneObject(type, 1));
        sceneObjects[0].position = glm::vec3(0.0f, getDefaultPositionForObject(1, type).y, 0.0f);
        selectedObjectIndex = 0;
    }
}

void randomizeSelectedObjectColor()
{
    if (sceneObjects.empty())
    {
        return;
    }
    selectedObjectIndex = std::clamp(selectedObjectIndex, 0, static_cast<int>(sceneObjects.size()) - 1);
    sceneObjects[selectedObjectIndex].color = randomColor();
}

glm::vec3 randomColor()
{
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<float> dist(0.25f, 0.95f);
    return glm::vec3(dist(rng), dist(rng), dist(rng));
}

glm::vec3 getDefaultPositionForObject(int index, ObjectType type)
{
    const std::array<glm::vec3, 6> positions = {
        glm::vec3(-2.2f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.2f, 0.0f, 0.0f),
        glm::vec3(-1.2f, 0.0f, -2.0f),
        glm::vec3(1.2f, 0.0f, -2.0f),
        glm::vec3(0.0f, 0.0f, 1.8f)};
    glm::vec3 position = positions[static_cast<std::size_t>(std::clamp(index, 0, 5))];
    if (type == ObjectType::Pyramid)
    {
        position.y = FLOOR_Y;
    }
    else if (type == ObjectType::Sphere)
    {
        position.y = FLOOR_Y + 0.88f;
    }
    else
    {
        position.y = FLOOR_Y + 0.68f;
    }
    return position;
}

glm::vec3 getDefaultScaleForObject(ObjectType type)
{
    if (type == ObjectType::Pyramid)
    {
        return glm::vec3(1.6f, 1.45f, 1.6f);
    }
    return glm::vec3(1.35f);
}

glm::vec3 getDefaultColorForObject(ObjectType type)
{
    if (type == ObjectType::Sphere)
    {
        return glm::vec3(0.0f, 0.75f, 0.68f);
    }
    if (type == ObjectType::Pyramid)
    {
        return glm::vec3(0.95f, 0.75f, 0.20f);
    }
    return glm::vec3(0.9f, 0.32f, 0.12f);
}

std::string objectTypeName(ObjectType type)
{
    if (type == ObjectType::Sphere)
    {
        return "Sphere";
    }
    if (type == ObjectType::Pyramid)
    {
        return "Pyramid";
    }
    return "Cube";
}

ObjectType nextObjectType(ObjectType type)
{
    if (type == ObjectType::Cube)
    {
        return ObjectType::Sphere;
    }
    if (type == ObjectType::Sphere)
    {
        return ObjectType::Pyramid;
    }
    return ObjectType::Cube;
}

void toggleFullscreen(GLFWwindow *window)
{
    isFullscreen = !isFullscreen;
    if (isFullscreen)
    {
        glfwGetWindowPos(window, &windowedX, &windowedY);
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
    else
    {
        windowedWidth = static_cast<int>(SCR_WIDTH);
        windowedHeight = static_cast<int>(SCR_HEIGHT);
        glfwSetWindowMonitor(window, nullptr, windowedX, windowedY, windowedWidth, windowedHeight, 0);
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void)window;
    framebufferWidth = width;
    framebufferHeight = height;
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    (void)xoffset;
    double mouseX = 0.0;
    double mouseY = 0.0;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    int winW = 1;
    int winH = 1;
    glfwGetWindowSize(window, &winW, &winH);
    double fbMouseX = mouseX * static_cast<double>(framebufferWidth) / static_cast<double>(std::max(1, winW));

    if (fbMouseX >= sceneWidth)
    {
        uiScrollOffset = std::clamp(uiScrollOffset - static_cast<float>(yoffset) * 42.0f, 0.0f, 520.0f);
    }
}

void updateLayout(GLFWwindow *window)
{
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    panelWidth = std::max(PANEL_MIN_WIDTH, static_cast<int>(framebufferWidth * 0.28f));
    panelWidth = std::clamp(static_cast<int>(framebufferWidth * 0.26f), PANEL_MIN_WIDTH, PANEL_MAX_WIDTH);
    panelWidth = std::min(panelWidth, std::max(PANEL_MIN_WIDTH, framebufferWidth - 520));
    sceneWidth = std::max(1, framebufferWidth - panelWidth);
}

void printControls()
{
    std::cout << "Controls:\n"
              << "ESC: Exit\n"
              << "Arrow keys: Move light on X/Y axes\n"
              << "Z / X: Move light on Z axis\n"
              << "W / S: Move camera forward / backward\n"
              << "Q / E: Decrease / increase shininess\n"
              << "P: Phong mode\n"
              << "B: Blinn-Phong mode\n"
              << "V: PBR mode\n"
              << "1: Ambient only\n"
              << "2: Diffuse only\n"
              << "3: Specular only\n"
              << "4: Full lighting\n"
              << "O: Toggle Shadow Mapping\n"
              << "A: Toggle attenuation\n"
              << "G: Toggle Global Illumination approximation\n"
              << "T: Toggle auto-orbit light\n"
              << "R: Reset scene\n"
              << "M: Next material preset\n"
              << "C: Select Cube\n"
              << "H: Select Sphere\n"
              << "Y: Select Pyramid\n"
              << "= / keypad +: Add selected object type\n"
              << "Delete: Remove selected object\n"
              << "N: Randomize selected object color\n"
              << "[ / ]: Decrease / increase shadow strength\n"
              << "I: Toggle AI-GI Lite\n"
              << "K / L: Decrease / increase AI-GI strength\n"
              << "F11: Toggle fullscreen\n";
}

void drawScene(const Shader &shader, bool depthOnly)
{
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    if (!depthOnly)
    {
        shader.setVec3("objectColor", MATERIAL_PRESETS[currentMaterialIndex].floorColor);
        shader.setBool("isFloor", true);
    }
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    if (!depthOnly)
    {
        shader.setBool("isFloor", false);
    }

    const float halfRoomWidth = ROOM_WIDTH * 0.5f;
    const float halfRoomDepth = ROOM_DEPTH * 0.5f;
    const float roomCenterY = FLOOR_Y + ROOM_HEIGHT * 0.5f;
    const float wallThickness = 0.12f;

    model = glm::mat4(1.0f);
    const float sideWallCenterZ = -halfRoomDepth + SIDE_WALL_DEPTH * 0.5f;
    model = glm::translate(model, glm::vec3(-halfRoomWidth - wallThickness * 0.5f, roomCenterY, sideWallCenterZ));
    model = glm::scale(model, glm::vec3(wallThickness, ROOM_HEIGHT, SIDE_WALL_DEPTH));
    shader.setMat4("model", model);
    if (!depthOnly)
    {
        shader.setVec3("objectColor", glm::vec3(0.75f, 0.12f, 0.10f));
    }
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(halfRoomWidth + wallThickness * 0.5f, roomCenterY, sideWallCenterZ));
    model = glm::scale(model, glm::vec3(wallThickness, ROOM_HEIGHT, SIDE_WALL_DEPTH));
    shader.setMat4("model", model);
    if (!depthOnly)
    {
        shader.setVec3("objectColor", glm::vec3(0.10f, 0.22f, 0.85f));
    }
    glDrawArrays(GL_TRIANGLES, 0, 36);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, roomCenterY, -halfRoomDepth - wallThickness * 0.5f));
    model = glm::scale(model, glm::vec3(ROOM_WIDTH + wallThickness * 2.0f, ROOM_HEIGHT, wallThickness));
    shader.setMat4("model", model);
    if (!depthOnly)
    {
        shader.setVec3("objectColor", glm::vec3(0.22f, 0.24f, 0.28f));
    }
    glDrawArrays(GL_TRIANGLES, 0, 36);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, FLOOR_Y + ROOM_HEIGHT + wallThickness * 0.5f, 0.0f));
    model = glm::scale(model, glm::vec3(ROOM_WIDTH + wallThickness * 2.0f, wallThickness, ROOM_DEPTH + wallThickness));
    shader.setMat4("model", model);
    if (!depthOnly)
    {
        shader.setVec3("objectColor", glm::vec3(0.16f, 0.18f, 0.21f));
    }
    glDrawArrays(GL_TRIANGLES, 0, 36);

    for (const SceneObject &object : sceneObjects)
    {
        if (object.visible)
        {
            drawObject(object, shader, depthOnly);
        }
    }
}

void drawObject(const SceneObject &object, const Shader &shader, bool depthOnly)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, object.position);
    model = glm::rotate(model, glm::radians(object.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, object.scale);
    shader.setMat4("model", model);
    if (!depthOnly)
    {
        shader.setVec3("objectColor", object.color);
    }

    if (object.type == ObjectType::Sphere)
    {
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, nullptr);
    }
    else if (object.type == ObjectType::Pyramid)
    {
        glBindVertexArray(pyramidVAO);
        glDrawArrays(GL_TRIANGLES, 0, 18);
    }
    else
    {
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void generateSphere(float radius,
                    int sectorCount,
                    int stackCount,
                    std::vector<float> &vertices,
                    std::vector<unsigned int> &indices)
{
    const float PI = 3.14159265359f;

    for (int i = 0; i <= stackCount; ++i)
    {
        float stackAngle = PI / 2.0f - static_cast<float>(i) * PI / static_cast<float>(stackCount);
        float xy = radius * std::cos(stackAngle);
        float z = radius * std::sin(stackAngle);

        for (int j = 0; j <= sectorCount; ++j)
        {
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

    for (int i = 0; i < stackCount; ++i)
    {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices.push_back(static_cast<unsigned int>(k1));
                indices.push_back(static_cast<unsigned int>(k2));
                indices.push_back(static_cast<unsigned int>(k1 + 1));
            }

            if (i != stackCount - 1)
            {
                indices.push_back(static_cast<unsigned int>(k1 + 1));
                indices.push_back(static_cast<unsigned int>(k2));
                indices.push_back(static_cast<unsigned int>(k2 + 1));
            }
        }
    }
}

std::string lightingModeName()
{
    switch (lightingMode)
    {
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

std::string renderModeName()
{
    if (renderMode == 2)
    {
        return "PBR";
    }
    if (renderMode == 1)
    {
        return "Blinn-Phong";
    }
    return "Phong";
}

void drawControlPanel(UI &ui, GLFWwindow *window)
{
    (void)window;
    const float panelX = static_cast<float>(sceneWidth);
    const float panelY = 0.0f;
    const float panelW = static_cast<float>(panelWidth);
    const float padding = 28.0f;
    const float contentX = panelX + padding;
    const float contentW = panelW - padding * 2.0f;
    float cursorY = panelY + 28.0f - uiScrollOffset;

    auto section = [&](const std::string &title)
    {
        cursorY += 10.0f;
        ui.separator(contentX, cursorY, contentW);
        cursorY += 14.0f;
        ui.text(contentX, cursorY, title, glm::vec4(0.88f, 0.94f, 0.95f, 1.0f), 2.05f);
        cursorY += 31.0f;
    };

    ui.panel();
    ui.text(contentX, cursorY, "CONTROL PANEL", glm::vec4(0.0f, 0.85f, 0.85f, 1.0f), 3.25f);
    cursorY += 43.0f;
    ui.text(contentX, cursorY, "PHONG LIGHTING LAB", glm::vec4(0.78f, 0.88f, 0.90f, 1.0f), 2.15f);
    ui.text(panelX + panelW - 118.0f, panelY + 14.0f, "SCROLL", glm::vec4(0.45f, 0.55f, 0.57f, 1.0f), 1.35f);
    cursorY += 28.0f;

    section("SHADING MODEL");
    const float gap = 10.0f;
    const float buttonW = (contentW - gap * 2.0f) / 3.0f;
    if (ui.button(contentX, cursorY, buttonW, 42.0f, "PHONG", renderMode == 0))
        renderMode = 0;
    if (ui.button(contentX + buttonW + gap, cursorY, buttonW, 42.0f, "BLINN", renderMode == 1))
        renderMode = 1;
    if (ui.button(contentX + (buttonW + gap) * 2.0f, cursorY, buttonW, 42.0f, "PBR", renderMode == 2))
        renderMode = 2;
    cursorY += 48.0f;

    section("OBJECT SELECTION");
    if (ui.button(contentX, cursorY, 150.0f, 38.0f, multiObjectMode ? "MULTIPLE" : "SINGLE", multiObjectMode))
    {
        multiObjectMode = !multiObjectMode;
        if (!multiObjectMode && !sceneObjects.empty())
        {
            selectedObjectIndex = std::clamp(selectedObjectIndex, 0, static_cast<int>(sceneObjects.size()) - 1);
            SceneObject selected = sceneObjects[selectedObjectIndex];
            sceneObjects.clear();
            selected.position = getDefaultPositionForObject(1, selected.type);
            selected.position.x = 0.0f;
            sceneObjects.push_back(selected);
            selectedObjectIndex = 0;
            selectedObjectType = selected.type;
        }
    }
    if (ui.button(contentX + 162.0f, cursorY, 58.0f, 38.0f, "+", false))
    {
        addSceneObject(selectedObjectType);
    }
    cursorY += 43.0f;

    if (ui.button(contentX, cursorY, 185.0f, 38.0f, objectTypeName(selectedObjectType), false))
    {
        selectedObjectType = nextObjectType(selectedObjectType);
        if (!multiObjectMode)
        {
            setSingleObjectType(selectedObjectType);
        }
    }
    if (ui.button(contentX + 198.0f, cursorY, 126.0f, 38.0f, "ADD", false))
    {
        addSceneObject(selectedObjectType);
    }
    cursorY += 43.0f;

    if (ui.button(contentX, cursorY, 116.0f, 36.0f, "RANDOM", false))
    {
        randomizeSelectedObjectColor();
    }
    if (ui.button(contentX + 126.0f, cursorY, 108.0f, 36.0f, "REMOVE", false))
    {
        removeSelectedObject();
    }
    if (ui.button(contentX + 244.0f, cursorY, 96.0f, 36.0f, "CLEAR", false))
    {
        clearSceneObjects();
    }
    cursorY += 43.0f;

    ui.text(contentX, cursorY, "OBJECTS", glm::vec4(0.58f, 0.68f, 0.72f, 1.0f), 1.55f);
    cursorY += 24.0f;
    const int objectRows = std::min(static_cast<int>(sceneObjects.size()), 6);
    for (int i = 0; i < objectRows; ++i)
    {
        const bool selected = i == selectedObjectIndex;
        std::string label = std::to_string(i + 1) + " " + sceneObjects[i].name;
        if (ui.button(contentX, cursorY, contentW - 48.0f, 32.0f, label, selected))
        {
            selectedObjectIndex = i;
            selectedObjectType = sceneObjects[i].type;
        }
        if (ui.button(contentX + contentW - 40.0f, cursorY, 40.0f, 32.0f, sceneObjects[i].visible ? "ON" : "OFF", sceneObjects[i].visible))
        {
            sceneObjects[i].visible = !sceneObjects[i].visible;
        }
        cursorY += 36.0f;
    }
    cursorY += 10.0f;

    section("LIGHT COMPONENTS");
    if (ui.checkbox(contentX, cursorY, "AMBIENT", enableAmbient))
        enableAmbient = !enableAmbient;
    cursorY += 31.0f;
    if (ui.checkbox(contentX, cursorY, "DIFFUSE", enableDiffuse))
        enableDiffuse = !enableDiffuse;
    cursorY += 31.0f;
    if (ui.checkbox(contentX, cursorY, "SPECULAR", enableSpecular))
        enableSpecular = !enableSpecular;
    cursorY += 21.0f;

    section("SHADOW MAPPING");
    if (ui.checkbox(contentX, cursorY, "SHADOW", enableShadow))
        enableShadow = !enableShadow;
    cursorY += 33.0f;
    ui.slider(contentX, cursorY, contentW, "SHADOW STRENGTH", shadowStrength, 0.0f, 1.0f);
    cursorY += 45.0f;

    section("GLOBAL ILLUMINATION");
    if (ui.checkbox(contentX, cursorY, "GI APPROX", enableGI))
        enableGI = !enableGI;
    cursorY += 33.0f;
    ui.slider(contentX, cursorY, contentW, "GI STRENGTH", giStrength, 0.0f, 2.0f);
    cursorY += 45.0f;
    ui.slider(contentX, cursorY, contentW, "COLOR BLEED", colorBleedingStrength, 0.0f, 2.0f);
    cursorY += 45.0f;
    ui.slider(contentX, cursorY, contentW, "BOUNCE LIGHT", bounceStrength, 0.0f, 2.0f);
    cursorY += 39.0f;

    section("PBR MATERIAL");
    ui.slider(contentX, cursorY, contentW, "METALLIC", metallic, 0.0f, 1.0f);
    cursorY += 45.0f;
    ui.slider(contentX, cursorY, contentW, "ROUGHNESS", roughness, 0.04f, 1.0f);
    cursorY += 45.0f;
    ui.slider(contentX, cursorY, contentW, "AO", ao, 0.0f, 1.0f);
    cursorY += 45.0f;
    if (ui.button(contentX, cursorY, 185.0f, 38.0f, "NEXT MATERIAL", false))
    {
        applyMaterialPreset((currentMaterialIndex + 1) % MATERIAL_PRESETS.size());
    }
    ui.text(contentX + 205.0f, cursorY + 10.0f, MATERIAL_PRESETS[currentMaterialIndex].name, glm::vec4(0.75f, 0.84f, 0.86f, 1.0f), 1.65f);
    cursorY += 40.0f;

    section("AI-GI LITE");
    if (ui.checkbox(contentX, cursorY, "AI-GI LITE", enableAIGI))
        enableAIGI = !enableAIGI;
    cursorY += 33.0f;
    ui.slider(contentX, cursorY, contentW, "AI STRENGTH", aiStrength, 0.0f, 1.0f);
    cursorY += 45.0f;

    section("LIGHT / CAMERA");
    if (ui.checkbox(contentX, cursorY, "ORBIT LIGHT", autoOrbitLight))
        autoOrbitLight = !autoOrbitLight;
    cursorY += 31.0f;
    if (ui.checkbox(contentX, cursorY, "ATTENUATION", enableAttenuation))
        enableAttenuation = !enableAttenuation;
    cursorY += 33.0f;
    ui.slider(contentX, cursorY, contentW, "LIGHT INTENSITY", lightIntensity, 0.5f, 10.0f);
    cursorY += 45.0f;
    if (ui.button(contentX, cursorY, 122.0f, 38.0f, "RESET", false))
        resetScene();
}

void updateWindowTitle(GLFWwindow *window)
{
    std::ostringstream title;
    title << renderModeName()
          << " | Lighting: " << lightingModeName()
          << " | Shininess: " << static_cast<int>(shininess)
          << " | Roughness: " << std::fixed << std::setprecision(2) << roughness
          << " | Metallic: " << metallic
          << " | Shadow: " << (enableShadow ? "ON" : "OFF")
          << "(" << shadowStrength << ")"
          << " | GI: " << (enableGI ? "ON" : "OFF")
          << "(" << giStrength << ")"
          << " | AI-GI: " << (enableAIGI ? "ON" : "OFF")
          << "(" << aiStrength << ")"
          << " | Attenuation: " << (enableAttenuation ? "ON" : "OFF")
          << " | Orbit: " << (autoOrbitLight ? "ON" : "OFF")
          << " | Material: " << MATERIAL_PRESETS[currentMaterialIndex].name;
    glfwSetWindowTitle(window, title.str().c_str());
}
