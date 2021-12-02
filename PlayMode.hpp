#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>
#include <deque>
#include "CollisionManager.hpp"
#include "DrunkPerson.hpp"
#include "FrameBuffers.hpp"

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
	void SpiderAnimation();

	// ---- render related ----
	FrameBuffers fbs;

	// ---- camera control ----
	/**
	 * About camera move
	 * 
	 * - Camera will have a consistent offset & rotation between itself and player
	 *   by setting a constant value to camera transform in Blender.
	 *   Thus **YOU SHOULD NOT CHANGE CAMERA'S TRANSFORM HERE**.
	 *   You should move cameraAnchor to move the camera.
	 * 	 To adjust the constant offset, you can do that in Blender to have
	 *   a better result.
	 * 
	 * - Camera will follow player's movement smoothly. We use
	 *   `f(t) = 1 - a^t` function to smooth camera move, where `a`
	 *   is the damp factor below.
	 *   It is cameraAnchor that follows player.
	 * 	 Function is inspired by:
	 *   https://gamedev.stackexchange.com/questions/152465/smoothly-move-camera-to-follow-player
	 */
	Scene::Camera *camera = nullptr;
	Scene::Transform* cameraAnchor = nullptr;
	// damping factor (0, 1). Approximately camera will approach to desination in 10*a seconds.
	static constexpr float camera_move_damp = 0.01f;

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
	float aimingConeDistance = 5.0f;

	float xMouseTravel = 0.0f;
	float yMouseTravel = 0.0f;

	glm::vec2 mouse_pos; // mouse position in NDC

	//Using velocity only in the up-down axis
	float gravity = 200.0f;
	float jumpStrength = 50.0f;

	float timer = 120.0f;

	std::vector<Scene::Transform*> drunkTransforms;
	std::vector<Scene::Transform*> drunkPoses;

	bool win = false;
	bool lose = false;
	int numInTrash = 0;
	
	glm::vec3 get_mouse_position() const;

	std::shared_ptr< Sound::PlayingSample > bgm_loop;

	std::vector<Sound::Sample const*> cashiers;
	std::vector<Sound::Sample const*> throws;

	// Game Manager
	int score = 0;
};
