#pragma once
#include <glm/glm.hpp>

struct Particle {
    // xyz = position, w = density
    glm::vec4 position; 
    
    // xyz = velocity, w = pressure
    glm::vec4 velocity; 
};