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
		{
			platforms.push_back(&transform);
			collisionManager.registerAgent(&transform, true);
		}
		if (transform.name.find("TrashBinCollision") != std::string::npos)
		{
			trashBins.push_back(collisionManager.registerAgent(&transform, true));
		}
		std::cout << transform.name << std::endl;
	}

	std::cout << trashBins.size() << std::endl;

	for (Scene::Transform& transform : scene.transforms) {
		if (transform.name.find("DrunkPerson") != std::string::npos)
		{
			drunkPeople.push_back(new DrunkPerson(collisionManager.registerAgent(&transform, false), player));
		}
	}

	playerAgent = collisionManager.registerAgent(player, playerModel, false);

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const& evt, glm::uvec2 const& window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_LSHIFT || evt.key.keysym.sym == SDLK_RSHIFT)
		{
			shift.downs += 1;
			shift.pressed = true;
		}
	}
	else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_LSHIFT || evt.key.keysym.sym == SDLK_RSHIFT)
		{
			shift.pressed = false;
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
		shift.pressed = true;
	}
	else if (evt.type == SDL_MOUSEBUTTONUP) {
		shift.pressed = false;
    }	/*else if (evt.type == SDL_MOUSEMOTION) {
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
	}*/

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
		//glm::vec3 forward = frame[0];
		//glm::vec3 right = frame[1];
		glm::vec3 upVector = frame[2];

		//Update velocity
		if (playerAgent->collidedPrevFrame && up.downs > 0)
		{
			playerAgent->velocity += upVector * jumpStrength;
		}
		playerAgent->velocity.y = -move.x * 75.0f;
		playerAgent->velocity -= upVector * gravity * elapsed;
		player->position += playerAgent->velocity * elapsed;
		if (player->position.z < -30.0f)
		{
			player->position = glm::vec3(0.0f, 0.0f, 4.0f);
			playerAgent->velocity = glm::vec3(0.0f);
			//TODO: Reload the scene?
		}
		//Throw drunk person
		if (heldDrunkPerson && space.downs > 0)
		{
			heldDrunkPerson->launch(glm::vec3(playerAgent->velocity.x * 3.0f, playerAgent->velocity.y * 3.0f, playerAgent->velocity.z * 0.33f) + upVector * 45.0f);
			heldDrunkPerson = nullptr;
		}

		playerAgent->collidedPrevFrame = false;
	}

	//Time control
	if (shift.pressed)
	{
		timeFactor = std::max<float>(timeFactor - timeFactorChangeRate * elapsed, minTimeFactor);
	}
	else
	{
		timeFactor = std::min<float>(timeFactor + timeFactorChangeRate * elapsed, maxTimeFactor);
	}

	//Pick up drunk person if colliding with one
	if (heldDrunkPerson == nullptr)
	{
		for (DrunkPerson* drunkPerson : drunkPeople)
		{
			if (collisionManager.checkCollision(playerAgent, drunkPerson->agent) && drunkPerson->agent->collidedPrevFrame)
			{
				drunkPerson->pickUp();
				heldDrunkPerson = drunkPerson;
				break;
			}
		}
	}

	//Check drunk people collision with trashcans
	for (DrunkPerson* drunkPerson : drunkPeople)
	{
		for (CollisionAgent* trashCan : trashBins)
		{
			CollisionAxis outAxis = CollisionAxis::X;
			if (collisionManager.checkCollision(drunkPerson->agent, trashCan, outAxis) && outAxis == CollisionAxis::Z)
			{
				collisionManager.unregisterAgent(drunkPerson->agent, false);
				break;
			}
		}
	}

	//Update drunk people
	for (DrunkPerson* drunkPerson : drunkPeople)
	{
		drunkPerson->update(elapsed * timeFactor, gravity);
	}

	collisionManager.update();

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	space.downs = 0;
	shift.downs = 0;
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
