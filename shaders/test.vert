#version 450 core

struct Particle {
    vec4 position;
    vec4 velocity;
};

layout(std430, binding = 0) buffer ParticleBuffer {
    Particle particles[];
};

uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 pos = particles[gl_VertexID].position;
    
    // Apply camera transformations
    gl_Position = projection * view * vec4(pos.xyz, 1.0);
}