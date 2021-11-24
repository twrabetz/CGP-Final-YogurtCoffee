#version 330 core

uniform sampler2D POSITION_TEX;
uniform sampler2D NORMAL_TEX;
uniform sampler2D ALBEDO_TEX;
uniform int LIGHT_TYPE;
uniform vec3 LIGHT_LOCATION;
uniform vec3 LIGHT_DIRECTION;
uniform vec3 LIGHT_ENERGY;
uniform float LIGHT_CUTOFF;
uniform vec3 EYE;

out vec4 fragColor;

void main() {
    ivec2 screen_pos = ivec2(gl_FragCoord.xy);
    vec3 position = texelFetch(POSITION_TEX, screen_pos, 0).xyz;
    vec3 normal = texelFetch(NORMAL_TEX, screen_pos, 0).xyz;
    vec4 albedo = texelFetch(ALBEDO_TEX, screen_pos, 0);
    vec3 v = normalize(EYE - position);
    vec3 n = normalize(normal);
    vec3 l; // light direction
    vec3 h; // half vector
    float nl; // cosine of incident angle * attenuation
    float c;
    float dis2; // light distance square
    vec3 e; // light flux

    switch (LIGHT_TYPE) {
        case 0: // point light
            l = (LIGHT_LOCATION - position);
            dis2 = dot(l, l);
            l = normalize(l);
            h = normalize(l+v);
            nl = max(0., dot(n, l)) / max(1., dis2);
            e = nl * LIGHT_ENERGY;
            break;
        
        case 3: // directional light
            l = -LIGHT_DIRECTION;
            h = normalize(l+v);
            e = max(0., dot(n, l)) * LIGHT_ENERGY;
            break;
        
        default:
            discard; // Unreachable
            break;
    }
    
    vec3 ambient = albedo.rgb * 0.1; // hardcoded
    vec3 diffuse = e * albedo.rgb; // diffuse
    fragColor = vec4(ambient + diffuse, albedo.a);
}