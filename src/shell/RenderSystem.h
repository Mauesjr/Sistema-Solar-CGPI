#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <vector>
#include "core/Particle.h"
#include "shell/Shader.h"

class RenderSystem {
public:
    RenderSystem();
    ~RenderSystem();

    void init(const std::vector<Particle>& initialParticles);
    void draw(const Shader& shader, unsigned int particleCount) const;
    void drawBoundingBox(const Shader& boxShader) const;
    void resetParticles(const std::vector<Particle>& initialParticles);

private:
    unsigned int ssbo;
    
    // Sphere Geometry Buffers
    unsigned int sphereVao;
    unsigned int sphereVbo;
    unsigned int sphereEbo;
    unsigned int sphereIndexCount;
    
    // Bounding box Buffers
    unsigned int boxVao;
    unsigned int boxVbo;
    unsigned int boxEbo;
    
    void setupSphere();
    void setupBoundingBox();
};

#endif // RENDER_SYSTEM_H