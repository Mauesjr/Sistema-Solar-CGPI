#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCodeStr = loadShaderCode(vertexPath);
    std::string fragmentCodeStr = loadShaderCode(fragmentPath);

    const char* vShaderCode = vertexCodeStr.c_str();
    const char* fShaderCode = fragmentCodeStr.c_str();

    unsigned int vertex, fragment;

    // Compile Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // Compile Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // Link Shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // Clean up individual shaders as they are linked into the program now
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::Shader(const char* computePath) {
    std::string computeCodeStr = loadShaderCode(computePath);
    const char* cShaderCode = computeCodeStr.c_str();

    unsigned int compute;

    // Compile Compute Shader
    compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &cShaderCode, NULL);
    glCompileShader(compute);
    checkCompileErrors(compute, "COMPUTE");

    // Link Shader Program
    ID = glCreateProgram();
    glAttachShader(ID, compute);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(compute);
}

std::string Shader::loadShaderCode(const char* path) const {
    std::vector<std::string> candidates = { path };
    std::string parentPath = std::string("../") + path;
    if (path != parentPath) candidates.push_back(parentPath);

    for (const auto& candidate : candidates) {
        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            shaderFile.open(candidate);
            std::stringstream shaderStream;
            shaderStream << shaderFile.rdbuf();
            shaderFile.close();
            return shaderStream.str();
        } catch (std::ifstream::failure&) {
            continue;
        }
    }

    std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << path << std::endl;
    return {};
}

void Shader::use() const {
    glUseProgram(ID);
}

void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setUInt(const std::string &name, unsigned int value) const {
    glUniform1ui(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const {
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const {
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const {
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const {
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkCompileErrors(unsigned int shader, std::string type) const {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" 
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" 
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}