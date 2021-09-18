#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	Scene::Transform* player = nullptr;
	Scene::Transform* playerModel = nullptr;
	Scene::Transform* cameraAnchor = nullptr;
	std::vector<Scene::Transform*> platforms;

	float xMouseTravel = 0.0f;
	float yMouseTravel = 0.0f;

	//Using velocity only in the up-down axis
	float playerVelocity = 0.0f;
	float gravity = 200.0f;
	float jumpStrength = 75.0f;

	//Player "Squish" mechanism:
	float squishInForce = 0.0f;
	float squishOutForce = 0.0f;
	float squishCompression = 0.0f;
	float squishTransferRate = 350.0f;

	bool collidedPrevFrame = false;

	bool squishing() { return abs(squishCompression + squishInForce) >= 0.1f; }
	
	//camera:
	Scene::Camera *camera = nullptr;

};
