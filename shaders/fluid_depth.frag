#version 450 core

in vec4 WorldPos;
in float Speed;
in float Density;
in float Pressure;

uniform mat4 view;
uniform int colorMode;

out vec2 DepthSpeed;

void main() {
    vec3 viewPos = (view * WorldPos).xyz;
    float depth = -viewPos.z;
    float normalizedSpeed = clamp(Speed / 30.0, 0.0, 1.0);
    float dataY;
    if (colorMode == 2) {
        dataY = clamp(Density / 5.0, 0.0, 1.0);
    } else if (colorMode == 3) {
        dataY = clamp(Pressure / 500.0, 0.0, 1.0);
    } else {
        dataY = normalizedSpeed;
    }
    DepthSpeed = vec2(depth, dataY);
}
