#include "RenderSystem.h"
#include <glad/glad.h>
#include <cmath>
#include <cstddef>

const float PI = 3.14159265359f;

RenderSystem::RenderSystem() : ssbo(0), sphereVao(0), sphereVbo(0), sphereEbo(0), sphereIndexCount(0), boxVao(0), boxVbo(0), boxEbo(0) {
}

RenderSystem::~RenderSystem() {
    if (ssbo != 0) glDeleteBuffers(1, &ssbo);
    if (sphereVao != 0) glDeleteVertexArrays(1, &sphereVao);
    if (sphereVbo != 0) glDeleteBuffers(1, &sphereVbo);
    if (sphereEbo != 0) glDeleteBuffers(1, &sphereEbo);
    if (boxVao != 0) glDeleteVertexArrays(1, &boxVao);
    if (boxVbo != 0) glDeleteBuffers(1, &boxVbo);
    if (boxEbo != 0) glDeleteBuffers(1, &boxEbo);
}

void RenderSystem::init(const std::vector<Particle>& initialParticles) {
    // --- Particle SSBO Setup ---
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, initialParticles.size() * sizeof(Particle), initialParticles.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    setupSphere();
    setupBoundingBox();
}

void RenderSystem::setupSphere() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Low-poly sphere to maintain performance
    int sectors = 8;
    int stacks = 8;

    // Generate vertices and normals
    for (int i = 0; i <= stacks; ++i) {
        float V = i / (float)stacks;
        float phi = V * PI;
        for (int j = 0; j <= sectors; ++j) {
            float U = j / (float)sectors;
            float theta = U * (PI * 2.0f);
            
            float x = std::cos(theta) * std::sin(phi);
            float y = std::cos(phi);
            float z = std::sin(theta) * std::sin(phi);
            
            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            // Normal (For a unit sphere at the origin, the position is the normal)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    // Generate indices
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < sectors; ++j) {
            int first = (i * (sectors + 1)) + j;
            int second = first + sectors + 1;
            
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    sphereIndexCount = indices.size();

    // Upload to GPU
    glGenVertexArrays(1, &sphereVao);
    glGenBuffers(1, &sphereVbo);
    glGenBuffers(1, &sphereEbo);

    glBindVertexArray(sphereVao);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void RenderSystem::setupBoundingBox() {
    float minX = 0.0f, minY = 0.0f, minZ = 0.0f;
    float maxX = 40.0f, maxY = 40.0f, maxZ = 40.0f;

    float vertices[] = {
        minX, minY, minZ, maxX, minY, minZ, maxX, maxY, minZ, minX, maxY, minZ,
        minX, minY, maxZ, maxX, minY, maxZ, maxX, maxY, maxZ, minX, maxY, maxZ
    };

    unsigned int indices[] = {
        0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7
    };

    glGenVertexArrays(1, &boxVao);
    glGenBuffers(1, &boxVbo);
    glGenBuffers(1, &boxEbo);
    glBindVertexArray(boxVao);
    glBindBuffer(GL_ARRAY_BUFFER, boxVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void RenderSystem::draw(const Shader& shader, unsigned int particleCount) const {
    shader.use();
    glBindVertexArray(sphereVao);
    
    // Instanced rendering command
    glDrawElementsInstanced(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0, particleCount);
    
    glBindVertexArray(0);
}

void RenderSystem::drawBoundingBox(const Shader& boxShader) const {
    boxShader.use();
    glBindVertexArray(boxVao);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RenderSystem::resetParticles(const std::vector<Particle>& initialParticles) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, initialParticles.size() * sizeof(Particle), initialParticles.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}