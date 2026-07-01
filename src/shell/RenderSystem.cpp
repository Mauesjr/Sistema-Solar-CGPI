#include "RenderSystem.h"
#include <glad/glad.h>
#include <cstddef>

RenderSystem::RenderSystem() : ssbo(0), vao(0) {
}

RenderSystem::~RenderSystem() {
    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
    }
    if (ssbo != 0) {
        glDeleteBuffers(1, &ssbo);
    }
}

void RenderSystem::init(const std::vector<Particle>& initialParticles) {
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    
    // Allocate memory and copy initial particle data to the GPU
    glBufferData(GL_SHADER_STORAGE_BUFFER, initialParticles.size() * sizeof(Particle), initialParticles.data(), GL_DYNAMIC_DRAW);
    
    // Bind to index 0 so the compute shaders can access it via layout(std430, binding = 0)
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // OpenGL requires a bound VAO to execute draw calls, even when fetching data purely from an SSBO
    glGenVertexArrays(1, &vao);
}

void RenderSystem::draw(const Shader& shader, unsigned int particleCount) const {
    shader.use();
    glBindVertexArray(vao);
    
    // Render using instancing mechanics via glDrawArrays
    glDrawArrays(GL_POINTS, 0, particleCount);
    
    glBindVertexArray(0);
}