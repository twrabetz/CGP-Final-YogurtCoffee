#version 330 core
out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D TEXTURE;

void main()
{ 
    FragColor = texture(TEXTURE, texCoords);
}