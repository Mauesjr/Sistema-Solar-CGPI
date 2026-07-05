#version 450 core

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

void main() {
    // Simple directional lighting
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    
    // Calculate diffuse impact
    float diff = max(dot(normalize(Normal), lightDir), 0.0);
    
    // Fluid color (Water Blue)
    vec3 objectColor = vec3(0.15, 0.55, 0.95); 
    
    vec3 ambient = 0.3 * objectColor;
    vec3 diffuse = diff * objectColor;
    
    FragColor = vec4(ambient + diffuse, 1.0);
}