#version 330 core

uniform sampler2D POSITION_TEX;
uniform sampler2D NORMAL_TEX;
uniform sampler2D ALBEDO_TEX;
uniform int LIGHT_TYPE;
uniform vec3 LIGHT_LOCATION;
uniform vec3 LIGHT_DIRECTION;
uniform vec3 LIGHT_ENERGY;
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

    // Blinn-Phong terms
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    switch (LIGHT_TYPE) {
        case 0: // point light
            l = (LIGHT_LOCATION - position);
            dis2 = dot(l, l);
            l = normalize(l);
            h = normalize(l+v);
            nl = max(0., dot(n, l)) / max(1., dis2);
            e = nl * LIGHT_ENERGY;
            // point light will not produce ambient light
            diffuse = e * albedo.rgb * 0.7; // diffuse
            specular = pow(max(0., dot(n, h)), 5.) * e * 0.2;
            break;
        
        case 3: // directional light controls global illumination
            ambient = albedo.rgb * LIGHT_ENERGY; // hardcoded ambient lighting
            break;
        
        default:
            discard; // Unreachable
            break;
    }
    
    fragColor = vec4(ambient + diffuse + specular, albedo.a);
}