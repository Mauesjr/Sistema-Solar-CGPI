#version 450 core

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in float Speed;
in float Density;
in float Pressure;

uniform int colorMode;
uniform vec3 cameraPos;

vec3 heatmap(float t) {
    t = clamp(t, 0.0, 1.0);
    vec3 c1 = vec3(0.0, 0.0, 0.5);
    vec3 c2 = vec3(0.0, 0.5, 1.0);
    vec3 c3 = vec3(0.0, 1.0, 0.0);
    vec3 c4 = vec3(1.0, 1.0, 0.0);
    vec3 c5 = vec3(1.0, 0.0, 0.0);
    float step = 0.25;
    if (t < step) return mix(c1, c2, t / step);
    if (t < 2.0 * step) return mix(c2, c3, (t - step) / step);
    if (t < 3.0 * step) return mix(c3, c4, (t - 2.0 * step) / step);
    return mix(c4, c5, (t - 3.0 * step) / step);
}

void main() {
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    vec3 normal = normalize(Normal);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 objectColor;
    if (colorMode == 0) {
        objectColor = vec3(0.15, 0.55, 0.95);
    } else if (colorMode == 1) {
        objectColor = heatmap(Speed / 30.0);
    } else if (colorMode == 2) {
        objectColor = heatmap(Density / 5.0);
    } else {
        objectColor = heatmap(Pressure / 500.0);
    }

    vec3 ambient = 0.3 * objectColor;
    vec3 diffuse = diff * objectColor;

    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), 32.0);
    vec3 specular = vec3(1.0) * spec * 0.5;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}