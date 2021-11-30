#version 330 core

uniform sampler2D LIGHTING_TEX;
uniform sampler2D POSITION_TEX;

in vec2 texCoords;

out vec4 fragColor;

void main() {
    vec3 lighting = texture(LIGHTING_TEX, texCoords).rgb;

    // linear fog
    // since we know the scene is along Y axis and camera is in negative x
    // fog started at 0 and end at 200, capped to 0.9
    float fog_start = 0.;
    float fog_end = 200.;
    vec3 position = texture(POSITION_TEX, texCoords).rgb;
    float z = clamp(position.x, fog_start, fog_end);
    float fog_factor = (z - fog_start) / (fog_end - fog_start);
    float fog_alpha = mix(0., 0.9, fog_factor);
    lighting = mix(lighting, vec3(1., 1., 1.), fog_factor);

    // Tone Mapping ref: https://learnopengl.com/Advanced-Lighting/HDR
    vec3 mapped = lighting / (lighting + vec3(1.0));
    fragColor = vec4(mapped, 1.);
}