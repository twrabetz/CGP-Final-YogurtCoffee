#include "PostPassProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"

Load< PostPassProgram > post_pass_program(LoadTagEarly, [](){
    return new PostPassProgram();
});

PostPassProgram::PostPassProgram() {
    program = gl_compile_program(
        // vertex shader
        "#version 330 core\n"
        "layout (location = 0) in vec2 Position;\n"
        "layout (location = 1) in vec2 TexCoords;\n"
        "\n"
        "out vec2 texCoords;\n"
        "\n"
        "void main() {\n"
        "    gl_Position = vec4(Position.x, Position.y, 0., 1.);\n"
        "    texCoords = TexCoords;\n"
        "}\n"
        ,
        // fragment shader
        "#version 330 core\n"
        "\n"
        "uniform sampler2D LIGHTING_TEX;\n"
        "uniform sampler2D POSITION_TEX;\n"
        "\n"
        "in vec2 texCoords;\n"
        "\n"
        "out vec4 fragColor;\n"
        "\n"
        "void main() {\n"
        "    vec3 lighting = texture(LIGHTING_TEX, texCoords).rgb;\n"
        "\n"
        "    // linear fog\n"
        "    // since we know the scene is along Y axis and camera is in negative x\n"
        "    // fog started at 0 and end at 200, capped to 0.9\n"
        "    float fog_start = 0.;\n"
        "    float fog_end = 200.;\n"
        "    vec3 position = texture(POSITION_TEX, texCoords).rgb;\n"
        "    float z = clamp(position.x, fog_start, fog_end);\n"
        "    float fog_factor = (z - fog_start) / (fog_end - fog_start);\n"
        "    float fog_alpha = mix(0., 0.9, fog_factor);\n"
        "    lighting = mix(lighting, vec3(1., 1., 1.), fog_factor);\n"
        "\n"
        "    // Tone Mapping ref: https://learnopengl.com/Advanced-Lighting/HDR\n"
        "    vec3 mapped = lighting;\n"
        "    fragColor = vec4(mapped, 1.);\n"
        "}\n"
    );

    Position_vec3 = glGetAttribLocation(program, "Position"); // not necessary, but do Jim's pipeline a favor

    // bind texture locations
    GLuint LIGHTING_TEX_sampler2D = glGetUniformLocation(program, "LIGHTING_TEX");
    GLuint POSITION_TEX_sampler2D = glGetUniformLocation(program, "POSITION_TEX");

    glUseProgram(program);

    glUniform1i(LIGHTING_TEX_sampler2D, 0);
    glUniform1i(POSITION_TEX_sampler2D, 1);

    glUseProgram(0);
}

PostPassProgram::~PostPassProgram() {
    glDeleteProgram(program);
    program = 0;
}
