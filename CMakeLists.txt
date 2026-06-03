cmake_minimum_required(VERSION 3.20)

project(PhongLighting CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/Shader.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glfw
        GLEW::GLEW
        OpenGL::GL
)

# Copy shader files next to the .exe after build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)
