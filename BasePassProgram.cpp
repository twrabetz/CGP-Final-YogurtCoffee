#include "BasePassProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"

Scene::Drawable::Pipeline base_pass_program_pipeline;

Load< BasePassProgram > base_pass_program(LoadTagEarly, []() -> BasePassProgram const * {
	BasePassProgram *ret = new BasePassProgram();

	//----- build the pipeline template -----
	base_pass_program_pipeline.program = ret->program;

	base_pass_program_pipeline.OBJECT_TO_CLIP_mat4 = ret->OBJECT_TO_CLIP_mat4;
	base_pass_program_pipeline.OBJECT_TO_LIGHT_mat4x3 = ret->OBJECT_TO_LIGHT_mat4x3;
	base_pass_program_pipeline.NORMAL_TO_LIGHT_mat3 = ret->NORMAL_TO_LIGHT_mat3;

	return ret;
});

BasePassProgram::BasePassProgram() {
	//Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
	program = gl_compile_program(
		//vertex shader:
        "#version 330 core\n"
        "\n"
        "uniform mat4 OBJECT_TO_CLIP;\n"
        "// this is actually not the light space but world space\n"
        "// however to be compatible with the current scene rendering framework\n"
        "// we keep the name\n"
        "uniform mat4x3 OBJECT_TO_LIGHT;\n"
        "uniform mat3 NORMAL_TO_LIGHT;\n"
        "\n"
        "layout(location = 0) in vec4 Position;\n"
        "layout(location = 1) in vec3 Normal;\n"
        "layout(location = 2) in vec4 Color;\n"
        "layout(location = 3) in vec2 TexCoord;\n"
        "\n"
        "out vec3 position;\n"
        "out vec3 normal;\n"
        "out vec4 color;\n"
        "out vec2 texCoord;\n"
        "\n"
        "void main() {\n"
        "    gl_Position = OBJECT_TO_CLIP * Position;\n"
        "    position = OBJECT_TO_LIGHT * Position;\n"
        "    normal = NORMAL_TO_LIGHT * Normal;\n"
        "    color = Color;\n"
        "    texCoord = TexCoord;\n"
        "}\n"
        "\n"
	,
		//fragment shader:
        "#version 330 core\n"
		"\n"
		"in vec3 position;\n"
		"in vec3 normal;\n"
		"in vec4 color;\n"
		"in vec2 texCoord;\n"
		"\n"
		"layout(location = 0) out vec3 gPosition;\n"
		"layout(location = 1) out vec3 gNormal;\n"
		"layout(location = 2) out vec4 gAlbedo;\n"
		"\n"
		"void main() {\n"
		"    gPosition = position;\n"
		"    gNormal = normalize(normal);\n"
		"    gAlbedo = color;\n"
		"}\n"
		"\n"
	);
	//As you can see above, adjacent strings in C/C++ are concatenated.
	// this is very useful for writing long shader programs inline.

	//look up the locations of vertex attributes:
	Position_vec4 = glGetAttribLocation(program, "Position");
	Normal_vec3 = glGetAttribLocation(program, "Normal");
	Color_vec4 = glGetAttribLocation(program, "Color");
	TexCoord_vec2 = glGetAttribLocation(program, "TexCoord");

	//look up the locations of uniforms:
	OBJECT_TO_CLIP_mat4 = glGetUniformLocation(program, "OBJECT_TO_CLIP");
	OBJECT_TO_LIGHT_mat4x3 = glGetUniformLocation(program, "OBJECT_TO_LIGHT");
	NORMAL_TO_LIGHT_mat3 = glGetUniformLocation(program, "NORMAL_TO_LIGHT");
}

BasePassProgram::~BasePassProgram() {
	glDeleteProgram(program);
	program = 0;
}
