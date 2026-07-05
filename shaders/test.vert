#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

struct Particle {
    vec4 position;
    vec4 velocity;
};

// The SSBO containing the positions of all 4,000 particles
layout(std430, binding = 0) buffer ParticleBuffer {
    Particle particles[];
};

uniform mat4 view;
uniform mat4 projection;
uniform float particleRadius;

out vec3 Normal;
out vec3 FragPos;

void main() {
    // Get the specific particle's world position using the instancing ID
    vec3 particleCenter = particles[gl_InstanceID].position.xyz;
    
    // Scale the base sphere by the radius and move it to the particle's position
    vec3 worldPos = (aPos * particleRadius) + particleCenter;
    
    FragPos = worldPos;
    Normal = aNormal;
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
}