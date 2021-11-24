#pragma once

#include "GL.hpp"
#include <glm/glm.hpp>

struct FrameBuffers {
    // textures
    GLuint position_tex = 0;
    GLuint normal_tex = 0;
    GLuint albedo_tex = 0;

    GLuint output_tex = 0;

    // render buffer
    GLuint depth_rb = 0;

    // frame buffers
    GLuint objects_fb = 0; // position, normal, albedo + depth for base pass
    GLuint lights_fb = 0; // lighting output + depth

    glm::uvec2 size{0}; // canvas/screen size

    // method to adjust tex/render buffer/frame buffer size
    void resize(glm::uvec2 const &drawable_size);
};
