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

private:
    unsigned int ssbo;
    unsigned int vao;
};

#endif // RENDER_SYSTEM_H