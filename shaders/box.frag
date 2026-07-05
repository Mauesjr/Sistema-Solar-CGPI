#version 450 core

out vec4 FragColor;

void main() {
    // Render the wireframe in a subtle white/grey
    FragColor = vec4(0.8, 0.8, 0.8, 0.5); 
}