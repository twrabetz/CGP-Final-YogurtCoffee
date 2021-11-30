#include "LightingPassProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"

Scene::Drawable::Pipeline lighting_pass_program_pipeline;

Load< LightingPassProgram > lighting_pass_program(LoadTagEarly, [](){
    LightingPassProgram *ret = new LightingPassProgram();

    lighting_pass_program_pipeline.program = ret->program;
    lighting_pass_program_pipeline.OBJECT_TO_CLIP_mat4 = ret->OBJECT_TO_CLIP_mat4;

    return ret;
});

LightingPassProgram::LightingPassProgram() {
    program = gl_compile_program(
        //vertex shader:
		"#version 330\n"
		"#line " STR(__LINE__) "\n"
        "\n"
        "uniform mat4 OBJECT_TO_CLIP;\n"
        "\n"
        "layout(location = 0) in vec3 Position;\n"
        "\n"
        "void main() {\n"
        "    gl_Position = OBJECT_TO_CLIP * vec4(Position, 1.f);\n"
        "}\n"
        ,
        "#version 330 core\n"
        "\n"
        "uniform sampler2D POSITION_TEX;\n"
        "uniform sampler2D NORMAL_TEX;\n"
        "uniform sampler2D ALBEDO_TEX;\n"
        "uniform int LIGHT_TYPE;\n"
        "uniform vec3 LIGHT_LOCATION;\n"
        "uniform vec3 LIGHT_DIRECTION;\n"
        "uniform vec3 LIGHT_ENERGY;\n"
        "uniform vec3 EYE;\n"
        "\n"
        "out vec4 fragColor;\n"
        "\n"
        "void main() {\n"
        "    ivec2 screen_pos = ivec2(gl_FragCoord.xy);\n"
        "    vec3 position = texelFetch(POSITION_TEX, screen_pos, 0).xyz;\n"
        "    vec3 normal = texelFetch(NORMAL_TEX, screen_pos, 0).xyz;\n"
        "    vec4 albedo = texelFetch(ALBEDO_TEX, screen_pos, 0);\n"
        "    vec3 v = normalize(EYE - position);\n"
        "    vec3 n = normalize(normal);\n"
        "    vec3 l; // light direction\n"
        "    vec3 h; // half vector\n"
        "    float nl; // cosine of incident angle * attenuation\n"
        "    float c;\n"
        "    float dis2; // light distance square\n"
        "    vec3 e; // light flux\n"
        "\n"
        "    // Blinn-Phong terms\n"
        "    vec3 ambient;\n"
        "    vec3 diffuse;\n"
        "    vec3 specular;\n"
        "\n"
        "    switch (LIGHT_TYPE) {\n"
        "        case 0: // point light\n"
        "            l = (LIGHT_LOCATION - position);\n"
        "            dis2 = dot(l, l);\n"
        "            l = normalize(l);\n"
        "            h = normalize(l+v);\n"
        "            nl = max(0., dot(n, l)) / max(1., dis2);\n"
        "            e = nl * LIGHT_ENERGY;\n"
        "            // point light will not produce ambient light\n"
        "            diffuse = e * albedo.rgb * 0.7; // diffuse\n"
        "            specular = pow(max(0., dot(n, h)), 5.) * e * 0.2;\n"
        "            break;\n"
        "        \n"
        "        case 3: // directional light controls global illumination\n"
        "            ambient = albedo.rgb * LIGHT_ENERGY; // hardcoded ambient lighting\n"
        "            break;\n"
        "        \n"
        "        default:\n"
        "            discard; // Unreachable\n"
        "            break;\n"
        "    }\n"
        "    \n"
        "    fragColor = vec4(ambient + diffuse + specular, albedo.a);\n"
        "}\n"
    );

    Position_vec3 = glGetAttribLocation(program, "Position"); // not necessary, but do Jim's pipeline a favor

    OBJECT_TO_CLIP_mat4 = glGetUniformLocation(program, "OBJECT_TO_CLIP");

    EYE_vec3 = glGetUniformLocation(program, "EYE");

	LIGHT_TYPE_int = glGetUniformLocation(program, "LIGHT_TYPE");
	LIGHT_LOCATION_vec3 = glGetUniformLocation(program, "LIGHT_LOCATION");
	LIGHT_DIRECTION_vec3 = glGetUniformLocation(program, "LIGHT_DIRECTION");
	LIGHT_ENERGY_vec3 = glGetUniformLocation(program, "LIGHT_ENERGY");

    // bind texture locations
    GLuint POSITION_TEX_sampler2D = glGetUniformLocation(program, "POSITION_TEX");
	GLuint NORMAL_TEX_sampler2D = glGetUniformLocation(program, "NORMAL_TEX");
	GLuint ALBEDO_TEX_sampler2D = glGetUniformLocation(program, "ALBEDO_TEX");

    glUseProgram(program);

    glUniform1i(POSITION_TEX_sampler2D, 0);
    glUniform1i(NORMAL_TEX_sampler2D, 1);
    glUniform1i(ALBEDO_TEX_sampler2D, 2);

    glUseProgram(0);

    // init lighting mesh
    {
        // use a cube as approximation
        lighting_sphere.type = GL_TRIANGLE_STRIP;
        static constexpr glm::vec3 cube_vertices[] = {
            {-1.0f,-1.0f, 1.0f},
            {-1.0f,-1.0f,-1.0f},
            { 1.0f,-1.0f, 1.0f},
            { 1.0f,-1.0f,-1.0f},
            { 1.0f, 1.0f, 1.0f},
            { 1.0f, 1.0f,-1.0f},
            {-1.0f, 1.0f, 1.0f},
            {-1.0f, 1.0f,-1.0f},
            {-1.0f,-1.0f, 1.0f},
            {-1.0f,-1.0f,-1.0f},
            {-1.0f,-1.0f,-1.0f},
            {-1.0f,-1.0f, 1.0f},
            {-1.0f,-1.0f, 1.0f},
            { 1.0f,-1.0f, 1.0f},
            {-1.0f, 1.0f, 1.0f},
            { 1.0f, 1.0f, 1.0f},
            { 1.0f, 1.0f, 1.0f},
            {-1.0f, 1.0f,-1.0f},
            {-1.0f, 1.0f,-1.0f},
            { 1.0f, 1.0f,-1.0f},
            {-1.0f,-1.0f,-1.0f},
            { 1.0f,-1.0f,-1.0f}
        };
        lighting_sphere.start = 0;
        lighting_sphere.count = sizeof(cube_vertices) / sizeof(glm::vec3);

        // create vao
        glGenVertexArrays(1, &lighting_sphere_vao);
        glBindVertexArray(lighting_sphere_vao);
        // upload vbo
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        // unbind
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // release vbo in advance. It will not be deleted immediately
        glDeleteBuffers(1, &vbo);
    }
}

LightingPassProgram::~LightingPassProgram() {
    glDeleteProgram(program);
    glDeleteVertexArrays(1, &lighting_sphere_vao);
    program = 0;
}
