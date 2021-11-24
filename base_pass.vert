#version 330 core

uniform mat4 OBJECT_TO_CLIP;
uniform mat4x3 OBJECT_TO_WORLD;
uniform mat3 NORMAL_TO_WORLD;

layout(location = 0) in vec4 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 TexCoord;

out vec3 position;
out vec3 normal;
out vec4 color;
out vec2 texCoord;

void main() {
    gl_Position = OBJECT_TO_CLIP * Position;
    position = OBJECT_TO_WORLD * Position;
    normal = NORMAL_TO_WORLD * Normal;
    color = Color;
    texCoord = TexCoord;
}