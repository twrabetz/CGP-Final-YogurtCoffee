#pragma once

#include "GL.hpp"
#include "Load.hpp"
#include "Scene.hpp"

struct PostPassProgram {
    PostPassProgram();
    ~PostPassProgram();

    GLuint program = 0;

    // light volume vertices
    GLuint Position_vec3 = -1U;

    //Uniform (per-invocation variable) locations:
};

extern Load< PostPassProgram > post_pass_program;
