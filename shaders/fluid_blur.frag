#version 450 core

in vec2 TexCoords;

uniform sampler2D depthTexture;
uniform bool horizontal;
uniform float blurRadius;

out vec2 Result;

float gaussian(float x, float sigma) {
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

void main() {
    vec2 texelSize = 1.0 / textureSize(depthTexture, 0);
    float sigma = blurRadius;
    vec2 sum = vec2(0.0);
    float weightSum = 0.0;
    int radius = int(ceil(blurRadius * 2.0));

    for (int x = -radius; x <= radius; x++) {
        vec2 offset = horizontal ? vec2(x * texelSize.x, 0.0) : vec2(0.0, x * texelSize.y);
        vec2 d = texture(depthTexture, TexCoords + offset).rg;
        float w = gaussian(float(x), sigma);
        sum += d * w;
        weightSum += w;
    }

    Result = sum / weightSum;
}
