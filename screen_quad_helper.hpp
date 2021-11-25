/**
 * @file screen_quad_helper.hpp
 * @author Harrison Chen (chenhao@andrew.cmu.edu)
 * @brief Helper to create a screen-sized quad
 * @version 0.1
 * @date 2021-11-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include "GL.hpp"

struct ScreenQuad
{
    ScreenQuad();
    ~ScreenQuad();

    GLuint vao = -1U;

    GLuint copy_program = -1U;
};
