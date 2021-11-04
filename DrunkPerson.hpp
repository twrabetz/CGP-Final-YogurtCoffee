#include "Scene.hpp"
#include "CollisionManager.hpp"

struct DrunkPerson
{

	glm::vec3 pickupOffset = glm::vec3(0, 0, 4.0f);
	float lerpSpeed = 5.0f;

	CollisionAgent* agent = nullptr;
	Scene::Transform* target = nullptr;

	DrunkPerson(CollisionAgent* agent, Scene::Transform* target);

	void update(float elapsed, float gravity);
	
	void pickUp();

	void launch(glm::vec3 velocity);
};