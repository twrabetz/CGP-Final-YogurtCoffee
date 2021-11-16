#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>
#include <deque>
#include "CollisionManager.hpp"
#include "DrunkPerson.hpp"

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
	void SpiderAnimation();

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
		uint8_t ups = 0;
	} left, right, down, up, space, mouse;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	Scene::Transform* player = nullptr;
	Scene::Transform* playerModel = nullptr;
	Scene::Transform* cameraAnchor = nullptr;
	std::vector<Scene::Transform*> platforms;

	Scene::Transform* SpiderRobot = nullptr;
	Scene::Transform* LFLegs_03 = nullptr;
	glm::quat LFLegs_03_rotation;
	Scene::Transform* LMLegs_03 = nullptr;
	glm::quat LMLegs_03_rotation;
	Scene::Transform* LBLegs_03 = nullptr;
	glm::quat LBLegs_03_rotation;

	float wobble = 0.0f;

	CollisionAgent* playerAgent;
	DrunkPerson* heldDrunkPerson = nullptr;

	float timeFactor = 1.0f;
	float maxTimeFactor = 1.0f;
	float minTimeFactor = 0.125f;
	float timeFactorChangeRate = 4.0f;

	std::vector<DrunkPerson*> drunkPeople;
	std::vector<CollisionAgent*> trashBins; //TODO: register the trashbin to code

	CollisionManager collisionManager = CollisionManager();

	Scene::Transform* aimingCone = nullptr;
	float aimingConeDistance = 3.5f;

	float xMouseTravel = 0.0f;
	float yMouseTravel = 0.0f;

	glm::vec2 mouse_pos; // mouse position in NDC

	//Using velocity only in the up-down axis
	float gravity = 200.0f;
	float jumpStrength = 50.0f;
	
	//camera:
	Scene::Camera *camera = nullptr;

	glm::vec3 get_mouse_position() const;

	// Game Manager
	int score = 0;
};
