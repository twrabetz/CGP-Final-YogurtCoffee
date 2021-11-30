#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"
#include "BasePassProgram.hpp"
#include "LightingPassProgram.hpp"
#include "screen_quad_helper.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "Sound.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <random>
#include <sstream>
#include <iostream>

/** Load render related components **/
// mesh buffer
GLuint meshes_for_basepass_program = 0;
Load< MeshBuffer > meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("SquidgeBall.pnct"));
	meshes_for_basepass_program = ret->make_vao_for_program(base_pass_program->program);
	return ret;
});
// screen quad in NDC space
Load< ScreenQuad > screen_quad(LoadTagDefault, [](){
	return new ScreenQuad();
});
/** Render related components finished **/

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

		//Transforms with names containing "Collider" will not be drawn
		if (transform->name.find("Collider") == std::string::npos)
		{

			scene.drawables.emplace_back(transform);
			Scene::Drawable& drawable = scene.drawables.back();

			drawable.pipeline = base_pass_program_pipeline;

			drawable.pipeline.vao = meshes_for_basepass_program;
			drawable.pipeline.type = mesh.type;
			drawable.pipeline.start = mesh.start;
			drawable.pipeline.count = mesh.count;
		}

		transform->radius = glm::vec3((mesh.max.x - mesh.min.x) / 2, (mesh.max.y - mesh.min.y) / 2, (mesh.max.z - mesh.min.z) / 2);
	});
});

Load< Sound::Sample > PartyMusic(LoadTagEarly, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("musics/Party_in_the_Castle.wav"));
});

Load< Sound::Sample > Cashier(LoadTagEarly, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("SFX/Cashier.opus"));
	});

Load< Sound::Sample > Throw1(LoadTagEarly, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("SFX/Flying_Moaning_001.opus"));
	});
Load< Sound::Sample > Throw2(LoadTagEarly, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("SFX/Flying_Moaning_002.opus"));
	});
Load< Sound::Sample > Throw3(LoadTagEarly, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("SFX/Flying_Moaning_003.opus"));
	});
Load< Sound::Sample > Throw4(LoadTagEarly, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("SFX/Flying_Moaning_004.opus"));
	});

PlayMode::PlayMode() : scene(*myScene) {

	for (Scene::Transform& transform : scene.transforms) {
		if (transform.name == "Player")
			player = &transform;
		if (transform.name == "CameraAnchor")
			cameraAnchor = &transform;
		if (transform.name.find("Icosphere") != std::string::npos)
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
		if (transform.name == "AimingCone")
		{
			aimingCone = &transform;
		}
		if (transform.name.find("PlatformCollider_TrashCan") != std::string::npos){
			//trashBins.push_back(collisionManager.registerAgent(&transform, false));
			trashBins.push_back(new CollisionAgent(&transform));
		}

		if(transform.name == "SpiderRobot") 
			SpiderRobot = &transform;
		if(transform.name == "LFLegs_03"){
			LFLegs_03 = &transform;
			LFLegs_03_rotation = LFLegs_03->rotation;
		}
		else if(transform.name == "LMLegs_03"){
			LMLegs_03 = &transform;
			LMLegs_03_rotation = LMLegs_03->rotation;
		}
		else if(transform.name == "LBLegs_03"){
			LBLegs_03 = &transform;
			LBLegs_03_rotation = LBLegs_03->rotation;
		}
	}

	std::cout << platforms.size() << std::endl;

	for (Scene::Transform& transform : scene.transforms) {
		if (transform.name.find("DrunkPerson") != std::string::npos)
		{
			drunkPeople.push_back(new DrunkPerson(collisionManager.registerAgent(&transform, false), player));
		}
	}

	playerAgent = collisionManager.registerAgent(player, playerModel, false);

	bgm_loop = Sound::loop(*PartyMusic);

	cashiers = { Cashier };
	throws = { Throw1, Throw2, Throw3, Throw4 };

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

PlayMode::~PlayMode() {
	for (CollisionAgent* C : trashBins)
		delete C;
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
	}
	else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		mouse.pressed = true;
	}
	else if (evt.type == SDL_MOUSEBUTTONUP) {
		mouse.pressed = false;
		mouse.ups += 1;
    }
	else if (evt.type == SDL_MOUSEMOTION) {
		// convert to NDC coordinates
		mouse_pos.x = 2 * float(evt.motion.x) / window_size.x - 1.f;
		mouse_pos.y = 1.f - 2 * float(evt.motion.y) / window_size.y;
	}

	return false;
}

std::string to_string(glm::vec3 input)
{
	return "(" + std::to_string(input.x) + ", " + std::to_string(input.y) + ", " + std::to_string(input.z) + ")";
}

void PlayMode::update(float elapsed) {

	//move the player:
	{
		wobble += elapsed;
		wobble -= std::floor(wobble);


		//combine inputs into a move:
		constexpr float PlayerSpeed = 15.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) {
			move.x = -1.0f;
			SpiderAnimation();
			if(SpiderRobot->scale.y < 0){
				SpiderRobot->scale.y = SpiderRobot->scale.y * -1.0f;
			}
		}
		if (!left.pressed && right.pressed) {
			move.x = 1.0f;
			SpiderAnimation();
			if(SpiderRobot->scale.y > 0){
				SpiderRobot->scale.y = SpiderRobot->scale.y * -1.0f;
			}
		}
		if (down.pressed && !up.pressed) {
			move.y = -1.0f;
		}
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed;

		glm::mat4x3 frame = playerModel->make_local_to_parent();
		//glm::vec3 forward = frame[0];
		//glm::vec3 right = frame[1];
		glm::vec3 upVector = frame[2];

		//Update velocity
		if (playerAgent->collidedPrevFrame && space.downs > 0 )
		{
			playerAgent->velocity += upVector * jumpStrength;
		}
		playerAgent->velocity.y = -move.x;
		if( move.y != 0 )
			playerAgent->velocity.x = move.y * 1.5f;
		playerAgent->velocity -= upVector * gravity * elapsed * timeFactor;
		player->position += playerAgent->velocity * elapsed * timeFactor;
		player->position.x = glm::clamp<float>(player->position.x, -2.89356f, 0.004437f);
		//std::cout << to_string(player->position) << std::endl;
		if (player->position.z < -30.0f)
		{
			player->position = glm::vec3(0.0f, 0.0f, 4.0f);
			playerAgent->velocity = glm::vec3(0.0f);
			//TODO: Reload the scene?
		}

		playerAgent->collidedPrevFrame = false;
	}

	//Aiming
	if (mouse.pressed && heldDrunkPerson != nullptr)
	{
		timeFactor = std::max<float>(timeFactor - timeFactorChangeRate * elapsed, minTimeFactor);
		glm::vec3 mousePos = get_mouse_position();
		glm::vec3 dir = glm::normalize(mousePos - player->position);
		aimingCone->position = player->position + dir * aimingConeDistance;
		aimingCone->rotation = glm::rotation(glm::vec3(0.0f, 0.0f, 1.0f), dir);
		aimingCone->drawn = true;
	}
	else
	{
		aimingCone->drawn = false;
		timeFactor = std::min<float>(timeFactor + timeFactorChangeRate * elapsed, maxTimeFactor);
	}

	//Throw drunk person
	if (heldDrunkPerson && mouse.ups > 0)
	{
		glm::vec3 mousePos = get_mouse_position();
		glm::vec3 dir = glm::normalize(mousePos - player->position);
		dir.x = 0;
		heldDrunkPerson->launch(dir * 100.0f);
		heldDrunkPerson = nullptr;
		Sound::play(*throws[rand() % throws.size()]);
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
			if (drunkPerson->agent->enabled && collisionManager.checkCollision(drunkPerson->agent, trashCan, outAxis) 
				&& outAxis == CollisionAxis::Z)
			{
				Sound::play(*cashiers[rand() % cashiers.size()]);
				std::cout << "hit";
				score ++;
				drunkPerson->inTrashBin(trashCan->positionTransform);
				heldDrunkPerson = nullptr;
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
	mouse.downs = 0;
	mouse.ups = 0;

	// ---- update camera position ----
	{
		float blend = std::pow(camera_move_damp, elapsed);
		cameraAnchor->position = glm::mix(
			player->position,
			cameraAnchor->position,
			blend
		);
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	/** Multipass rendering **/
	/**
	 * The rendering procedure consists of 3 passes
	 * 1. Base pass: render the visible gemoetry data to buffer for later use
	 * 2. Lighting pass: calculate lighting with geometry data and render to an output
	 * 3. Postprocessing pass: apply some VFX with shaders including:
	 *   - linear fog
	 *   - fake fog scattering
	 *   - HDR & tone mapping
	 *   - glow
	 */

	// ---- preparation ----
	fbs.resize(drawable_size); // update screen size

	// some camera related parameters
	glm::mat4 world_to_clip = camera->make_projection() * glm::mat4(camera->transform->make_world_to_local());
	glm::vec3 eye = camera->transform->make_local_to_world()[3];

	// ---- base pass ----
	glBindFramebuffer(GL_FRAMEBUFFER, fbs.objects_fb); // render to objects fb

	// clean buffer
	constexpr GLfloat zeros[4] = {0.f};
	glClearBufferfv(GL_COLOR, 0, zeros);
	glClearBufferfv(GL_COLOR, 1, zeros);
	glClearBufferfv(GL_COLOR, 2, zeros);
	glClear(GL_DEPTH_BUFFER_BIT);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// draw base pass
	scene.draw(*camera);

	// unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_ERRORS();

	// ---- lighting pass ----
	glBindFramebuffer(GL_FRAMEBUFFER, fbs.lights_fb); // output to lights fb
	// clean
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	// setting face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	// setting blend as addition
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(GL_FALSE); // do not modify z-buffer
	// setting program
	glUseProgram(lighting_pass_program->program);
	// use light volume
	//TODO: use different light meshes. For simplicity,
	// we just set a large radius for directional light
	glBindVertexArray(lighting_pass_program->lighting_sphere_vao);
	// some constant uniforms
	glUniform3fv(lighting_pass_program->EYE_vec3, 1, glm::value_ptr(eye));
	// bind g-buffer
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbs.position_tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fbs.normal_tex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fbs.albedo_tex);
	// render every light
	for (const auto &light : scene.lights) {
		glm::mat4 light_to_world = light.transform->make_local_to_world();
		// upload light specific uniforms
		glUniform3fv(lighting_pass_program->LIGHT_LOCATION_vec3, 1, glm::value_ptr(glm::vec3(light_to_world[3])));
		glUniform3fv(lighting_pass_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(light.energy));
		// upload light tpye specific uniforms
		switch (light.type) {
			case Scene::Light::Point: {
				glUniform1i(lighting_pass_program->LIGHT_TYPE_int, 0);
				// calc light volume radius
				constexpr float energy_thresh = 1.f / 256.f;
				// energy lower than threshold is considered no influence
				float R = std::sqrt(glm::compMax(light.energy) / energy_thresh);
				light_to_world = glm::scale(light_to_world, glm::vec3{R});
				break;
			}
			case Scene::Light::Directional: {
				glUniform1i(lighting_pass_program->LIGHT_TYPE_int, 3);
				glUniform3fv(lighting_pass_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(-light_to_world[2])));
				constexpr float fake_radius = 1024.f;
				light_to_world = glm::scale(light_to_world, glm::vec3{fake_radius});
				break;
			}
			default:
				// other light types not supported yet
				continue;
		}
		// send object to clip mat
		glUniformMatrix4fv(lighting_pass_program->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(world_to_clip * light_to_world));

		// draw
		const auto &mesh = lighting_pass_program->lighting_sphere;
		glDrawArrays(mesh.type, mesh.start, mesh.count);
	}
	// clean
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(0);

	glDepthMask(GL_TRUE);

	glDisable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_ERRORS();

	// ---- debug: draw buffer onto screen
	glClearColor(.2f, .2f, .2f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	// use copy program
	glUseProgram(screen_quad->copy_program);
	// bind screen quad vertices
	glBindVertexArray(screen_quad->vao);
	// bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbs.output_tex);
	// draw
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// clean
	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

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
		lines.draw_text("A/D moves left/right; S/W moves forward/backward; Space jumps; Mouse launches the person",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("A/D moves left/right; S/W moves forward/backward; Space jumps; Mouse launches the person",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		lines.draw_text("Score: ",
			glm::vec3(aspect - 6.0f * H, 0.9f - 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xbd, 0xd6, 0xff, 0x00));
		lines.draw_text(std::to_string(score),
			glm::vec3(aspect - 3.0f * H, 0.9f - 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xbd, 0xd6, 0xff, 0x00));
	}
}

glm::vec3 PlayMode::get_mouse_position() const {
	auto camera_to_world = camera->transform->make_local_to_world();
	// transform clip space endpoint to view space
	glm::mat4 inv_projection = glm::inverse(camera->make_projection());
	glm::vec4 mouse_view_endpoint = inv_projection * glm::vec4(mouse_pos.x, mouse_pos.y, -1.f, 1.f);
	// perspective divide
	glm::vec4 mouse_view_dir = mouse_view_endpoint / mouse_view_endpoint.w;
	// convert to homogeneous vector in view space, camera origin
	mouse_view_dir.w = 0;
	// view space to world space
	glm::vec3 mouse_world_dir = glm::normalize(camera_to_world * mouse_view_dir);
	// figure out the intersection with Y-Z plane. Our scene is built with X = 0.
	// shoot a ray O+tD, work out t
	auto camera_world_pos = camera_to_world[3];
	float t = -camera_world_pos.x / mouse_world_dir.x;
	// get intersection position in world space
	auto mouse_world_pos = camera_world_pos + t * mouse_world_dir;
	return mouse_world_pos;
}

void PlayMode::SpiderAnimation(){
	LFLegs_03->rotation = LFLegs_03_rotation * glm::angleAxis(
	glm::radians(30.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))), glm::vec3(1.0f, 1.0f, 0.0f));
	LMLegs_03->rotation = LMLegs_03_rotation * glm::angleAxis(
	glm::radians(30.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))), glm::vec3(0.0f, 1.0f, 0.0f));
	LBLegs_03->rotation = LBLegs_03_rotation * glm::angleAxis(
	glm::radians(30.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))), glm::vec3(-1.0f, 1.0f, 0.0f));
}
