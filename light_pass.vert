#version 330 core

uniform mat4 OBJECT_TO_CLIP;

layout(location = 0) in vec4 Position;

void main() {
    gl_Position = OBJECT_TO_CLIP * Position;
}