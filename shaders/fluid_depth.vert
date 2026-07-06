#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

struct Particle {
    vec4 position;
    vec4 velocity;
};

layout(std430, binding = 0) buffer ParticleBuffer {
    Particle particles[];
};

uniform mat4 view;
uniform mat4 projection;
uniform float particleRadius;

out vec4 WorldPos;
out float Speed;
out float Density;
out float Pressure;

void main() {
    vec3 particleCenter = particles[gl_InstanceID].position.xyz;
    vec3 worldPos = (aPos * particleRadius) + particleCenter;
    WorldPos = vec4(worldPos, 1.0);
    vec3 vel = particles[gl_InstanceID].velocity.xyz;
    Speed = length(vel);
    Density = particles[gl_InstanceID].position.w;
    Pressure = particles[gl_InstanceID].velocity.w;
    gl_Position = projection * view * vec4(worldPos, 1.0);
}
