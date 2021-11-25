#pragma once

#include "GL.hpp"
#include "Load.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"

struct LightingPassProgram {
    LightingPassProgram();
    ~LightingPassProgram();

    GLuint program = 0;

    // light volume vertices
    GLuint Position_vec3 = -1U;

    //Uniform (per-invocation variable) locations:
	GLuint OBJECT_TO_CLIP_mat4 = -1U;

    // lighting uniforms
    GLuint EYE_vec3 = -1U; //camera position in lighting space

	GLuint LIGHT_TYPE_int = -1U;
	GLuint LIGHT_LOCATION_vec3 = -1U;
	GLuint LIGHT_DIRECTION_vec3 = -1U;
	GLuint LIGHT_ENERGY_vec3 = -1U;

    // lighting volumes
    Mesh lighting_sphere; // unit sphere mesh
    GLuint lighting_sphere_vao = 0;
};

extern Load< LightingPassProgram > lighting_pass_program;
