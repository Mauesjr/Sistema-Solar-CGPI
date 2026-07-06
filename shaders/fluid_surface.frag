#version 450 core

in vec2 TexCoords;

uniform sampler2D depthTexture;
uniform mat4 projection;
uniform mat4 view;
uniform float fresnelPower;
uniform vec2 screenSize;
uniform int colorMode;

out vec4 FragColor;

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

vec3 reconstructWorldPos(vec2 uv, float depth, mat4 invProj, mat4 invView) {
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = invProj * clipPos;
    viewPos /= viewPos.w;
    vec4 worldPos = invView * viewPos;
    return worldPos.xyz;
}

void main() {
    vec2 texelSize = 1.0 / textureSize(depthTexture, 0);
    vec2 data = texture(depthTexture, TexCoords).rg;
    float depth = data.r;
    float dataValue = data.g;

    if (depth < 0.001) {
        FragColor = vec4(0.15, 0.15, 0.20, 1.0);
        return;
    }

    mat4 invProj = inverse(projection);
    mat4 invView = inverse(view);

    vec3 worldPos = reconstructWorldPos(TexCoords, depth, invProj, invView);

    // Reconstruct normals from depth buffer
    float dL = texture(depthTexture, TexCoords + vec2(-texelSize.x, 0.0)).r;
    float dR = texture(depthTexture, TexCoords + vec2(texelSize.x, 0.0)).r;
    float dD = texture(depthTexture, TexCoords + vec2(0.0, -texelSize.y)).r;
    float dU = texture(depthTexture, TexCoords + vec2(0.0, texelSize.y)).r;

    vec3 pL = reconstructWorldPos(TexCoords + vec2(-texelSize.x, 0.0), dL, invProj, invView);
    vec3 pR = reconstructWorldPos(TexCoords + vec2(texelSize.x, 0.0), dR, invProj, invView);
    vec3 pD = reconstructWorldPos(TexCoords + vec2(0.0, -texelSize.y), dD, invProj, invView);
    vec3 pU = reconstructWorldPos(TexCoords + vec2(0.0, texelSize.y), dU, invProj, invView);

    vec3 normal = normalize(cross(pR - pL, pU - pD));

    // Lighting
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    vec3 viewDir = normalize(-worldPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 baseColor;
    if (colorMode == 0) {
        baseColor = vec3(0.15, 0.55, 0.95);
    } else {
        baseColor = heatmap(dataValue);
    }

    vec3 ambient = 0.3 * baseColor;
    vec3 diffuse = diff * baseColor;

    // Fresnel
    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), fresnelPower);
    vec3 fresnelColor = mix(vec3(0.5, 0.8, 1.0), vec3(1.0), fresnel * 0.3);

    // Specular
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), 32.0);
    vec3 specular = vec3(1.0) * spec * 0.5;

    vec3 finalColor = ambient + diffuse + specular + fresnel * fresnelColor * 0.4;

    FragColor = vec4(finalColor, 1.0);
}
