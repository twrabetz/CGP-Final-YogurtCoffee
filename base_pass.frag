#version 330 core

in vec3 position;
in vec3 normal;
in vec4 color;
in vec2 texCoord;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedo;

void main() {
    gPosition = position;
    gNormal = normalize(normal);
    gAlbedo = color;
}