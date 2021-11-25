#include "screen_quad_helper.hpp"

#include <glm/glm.hpp>

#include "gl_compile_program.hpp"

ScreenQuad::ScreenQuad() {
    // create vertices & texcoords for screen-sized quad
    const float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    // screen quad VAO
    GLuint quadVBO;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    // unbind buffer
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // release VBO as pending deletion
    // it will be actually deleted after VAO is deleted
    glDeleteBuffers(1, &quadVBO);

    // create copy program
    copy_program = gl_compile_program(
        "#version 330 core\n"
        "layout (location = 0) in vec2 Position;\n"
        "layout (location = 1) in vec2 TexCoords;\n"
        "\n"
        "out vec2 texCoords;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(Position.x, Position.y, 0.0, 1.0); \n"
        "    texCoords = TexCoords;\n"
        "}\n"
        "\n",
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "\n"
        "in vec2 texCoords;\n"
        "\n"
        "uniform sampler2D TEXTURE;\n"
        "\n"
        "void main()\n"
        "{ \n"
        "    FragColor = texture(TEXTURE, texCoords);\n"
        "}\n"
    );
}

ScreenQuad::~ScreenQuad() {
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(copy_program);
    copy_program = -1U;
}
