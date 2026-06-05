#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

std::string Shader::readFile(const char* path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "ERROR::SHADER::FILE_READ_FAILED\n"
                  << "  Could not open shader file: " << path << "\n"
                  << "  Run the executable from the project root or make sure the shaders folder was copied next to the executable.\n";
        return "";
    }

    std::stringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode = readFile(vertexPath);
    std::string fragmentCode = readFile(fragmentPath);

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    if (vertexCode.empty()) {
        std::cerr << "ERROR::SHADER::VERTEX_SOURCE_EMPTY: " << vertexPath << "\n";
    }
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    checkCompileErrors(vertex, std::string("VERTEX ") + vertexPath);

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    if (fragmentCode.empty()) {
        std::cerr << "ERROR::SHADER::FRAGMENT_SOURCE_EMPTY: " << fragmentPath << "\n";
    }
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    checkCompileErrors(fragment, std::string("FRAGMENT ") + fragmentPath);

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, std::string("PROGRAM ") + vertexPath + " + " + fragmentPath);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use() const {
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::checkCompileErrors(unsigned int object, const std::string& type) {
    int success;
    char infoLog[1024];

    if (type.rfind("PROGRAM", 0) != 0) {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR: "
                      << type << "\n" << infoLog << std::endl;
        }
    } else {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR: "
                      << type << "\n" << infoLog << std::endl;
        }
    }
}
