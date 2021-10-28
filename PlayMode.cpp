#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

#include <iostream>

GLuint meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("SquidgeBall.pnct"));
	meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

std::string toString(glm::vec3 input)
{
	std::string res = "(";
	res.append(std::to_string(input.x));
	res.append(", ");
	res.append(std::to_string(input.y));
	res.append(", ");
	res.append(std::to_string(input.z));
	res.append(")");
	return res;
}

Load< Scene > myScene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("SquidgeBall.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

		transform->radius = glm::vec3((mesh.max.x - mesh.min.x) / 2, (mesh.max.y - mesh.min.y) / 2, (mesh.max.z - mesh.min.z) / 2);
	});
});

PlayMode::PlayMode() : scene(*myScene) {

	for (Scene::Transform& transform : scene.transforms) {
		if (transform.name == "Player")
			player = &transform;
		if (transform.name == "CameraAnchor")
			cameraAnchor = &transform;
		if (transform.name == "Icosphere")
		{
			playerModel = &transform;
			playerModel->rotation = glm::normalize(
				glm::angleAxis(0.0f * 1.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
		}
		if (transform.name.find("Platform") != std::string::npos)
			platforms.push_back(&transform);
	}

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			xMouseTravel += motion.x;
			yMouseTravel += motion.y;
			cameraAnchor->rotation = glm::normalize(
				glm::angleAxis(-xMouseTravel * 1.0f, glm::vec3(0.0f, 0.0f, 1.0f))
				* glm::angleAxis(-yMouseTravel * 1.0f, glm::vec3(1.0f, 0.0f, 0.0f)));
			playerModel->rotation = glm::normalize(
				glm::angleAxis(-xMouseTravel * 1.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//move the player:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 10.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = playerModel->make_local_to_parent();
		glm::vec3 right = frame[0];
		glm::vec3 forward = frame[1];
		glm::vec3 up = frame[2];

		//Update velocity
		/*if (!squishing())
		{
			playerVelocity += abs(squishOutForce) > 15.0f ? squishOutForce : 0.0f;
			squishOutForce = 0;
			if (collidedPrevFrame && space.downs > 0)
			{
				squishInForce -= jumpStrength;
			}
			playerVelocity -= gravity * elapsed;
			player->position += playerVelocity * up * elapsed;
			player->position -= move.x * right + move.y * forward;
			if (player->position.z < -30.0f)
			{
				player->position = glm::vec3(-2.0f, 0.0f, 4.0f);
				playerVelocity = 0.0f;
			}
		}
		else
		{
			//squishInForce -= gravity * elapsed;
			if (abs(squishInForce) > 0)
			{
				float sign = squishInForce / abs(squishInForce);
				float change = sign * std::min<float>(abs(squishInForce), squishTransferRate * elapsed);
				squishCompression += change;
				squishInForce -= change;
			}
			else if (abs(squishCompression) > 0)
			{
				float sign = squishCompression / abs(squishCompression);
				float change = sign * std::min<float>(abs(squishCompression), squishTransferRate * elapsed);
				squishCompression -= change;
				squishOutForce -= change * 0.75f;
			}
		}*/

		collidedPrevFrame = false;

		/*float oldZ = playerModel->scale.z;
		float scaleFactor = std::max<float>(1.0f - abs(squishCompression) / 40.0f, 0.1f);
		float sideScaleFactor = std::min<float>(1.0f + abs(squishCompression) / 100.0f, 1.33f);
		playerModel->scale = glm::vec3(sideScaleFactor, sideScaleFactor, scaleFactor);

		if (squishing())
		{
			player->position += (abs(squishCompression) > 0 ? (-squishCompression / abs(squishCompression)) : 1.0f) * up * playerModel->radius.z * (playerModel->scale.z - oldZ);
		}*/
	}

	//Handle collisions:
	auto checkCollision = [](glm::vec3 pos1, glm::vec3 pos2, glm::vec3 rad1, glm::vec3 rad2, glm::vec3 scale1, glm::vec3 scale2) -> bool
	{
		float xDiff = abs(pos1.x - pos2.x);
		float yDiff = abs(pos1.y - pos2.y);
		float zDiff = abs(pos1.z - pos2.z);
		bool result = xDiff <= rad1.x * scale1.x + rad2.x * scale2.x && yDiff <= rad1.y * scale1.y + rad2.y * scale2.y && zDiff <= rad1.z * scale1.z + rad2.z * scale2.z;
		//std::cout << "CheckCollision, Pos1: " << toString(pos1) << " Pos2: " << toString(pos2) << " Rad1: " << toString(rad1) << " Rad2: " 
		//	<< toString(rad2) << ": " << result << std::endl;
		return result;
	};

	auto combineFloat = [](float a, float b)
	{
		if (a == 0)
			return b;
		if (b == 0)
			return a;
		if (a < 0 && b >= 0 || a >= 0 && b < 0)
		{
			return a + b;
		}
		float res = (a / abs(a)) * std::max<float>(abs(a), abs(b));
		//std::cout << "CombineFloat combined A: " << a << " B: " << b << " to " << res << std::endl;
		return res;
	};

	auto combine = [combineFloat](glm::vec3 offset1, glm::vec3 offset2)
	{
		return glm::vec3(combineFloat(offset1.x, offset2.x), combineFloat(offset1.y, offset2.y), combineFloat(offset1.z, offset2.z));
	};

	//Make everything go ahead n bump everything else
	auto processCollision = [this, combine](glm::vec3 pos1, glm::vec3 pos2, glm::vec3 rad1, glm::vec3 rad2, glm::vec3 scale1, glm::vec3 scale2,
		glm::vec3& offset, float& playerVelocity)
	{
		float xDiff = abs(pos1.x - pos2.x);
		float yDiff = abs(pos1.y - pos2.y);
		float zDiff = abs(pos1.z - pos2.z);
		float xPush = std::max<float>(rad1.x * scale1.x + rad2.x * scale2.x - abs(xDiff), 0);
		float yPush = std::max<float>(rad1.y * scale1.y + rad2.y * scale2.y - abs(yDiff), 0);
		float zPush = std::max<float>(rad1.z * scale1.z + rad2.z * scale2.z - abs(zDiff), 0);
		if (zDiff > xDiff && zDiff > yDiff)
		{
			//this->collidedPrevFrame = true;
			//this->squishInForce += abs(playerVelocity) > 4.5f && (pos1.z >= pos2.z && playerVelocity < 0 || pos1.z < pos2.z && playerVelocity > 0) ? playerVelocity : 0.0f;
			playerVelocity = pos1.z >= pos2.z ? std::max<float>(0.0f, playerVelocity) : std::min<float>(0.0f, playerVelocity);
			offset = combine(offset, glm::vec3(0.0f, 0.0f, pos1.z >= pos2.z ? zPush : -zPush));
		}
		else if (xDiff > yDiff)
			offset = combine(offset, glm::vec3(pos1.x >= pos2.x ? xPush : -xPush, 0.0f, 0.0f));
		else
			offset = combine(offset, glm::vec3(0.0f, pos1.y >= pos2.y ? yPush : -yPush, 0.0f));
	};

	glm::vec3 playerOffset = glm::vec3(0.0f);

	if (!squishing())
	{
		for (Scene::Transform* platform : platforms)
		{
			if (checkCollision(player->position, platform->position, playerModel->radius, platform->radius, playerModel->scale, platform->scale))
				processCollision(player->position, platform->position, playerModel->radius, platform->radius, playerModel->scale, platform->scale,
					playerOffset, playerVelocity);
		}
	}

	//std::cout << "Player Offset X: " << playerOffset.x << " Y: " << playerOffset.y << " Z: " << playerOffset.z << std::endl;

	player->position += playerOffset;

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	space.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
