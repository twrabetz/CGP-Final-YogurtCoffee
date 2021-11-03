#pragma once

#include <glm/glm.hpp>
#include "Scene.hpp"

struct CollisionAgent
{
	Scene::Transform* positionTransform = nullptr;
	Scene::Transform* scaleTransform = nullptr;

	glm::vec3 velocity = glm::vec3(0.0f);

	glm::vec3 offset = glm::vec3(0.0f);

	bool collidedPrevFrame = false;

	bool enabled = true;

	CollisionAgent(Scene::Transform* newTransform)
	{
		positionTransform = newTransform;
		scaleTransform = newTransform;
	}

	CollisionAgent(Scene::Transform* newPositionTransform, Scene::Transform* newScaleTransform)
	{
		positionTransform = newPositionTransform;
		scaleTransform = newScaleTransform;
	}
};

struct CollisionManager
{

	~CollisionManager();

	//Handle collisions:
	bool checkCollision(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 rad1, glm::vec3 rad2, glm::vec3 scale1, glm::vec3 scale2);

	float combineFloat(float a, float b);

	glm::vec3 combine(glm::vec3 offset1, glm::vec3 offset2);

	//Make everything go ahead n bump everything else
	void processCollision(CollisionAgent* agent, glm::vec3 pos1, glm::vec3 pos2, glm::vec3 rad1, glm::vec3 rad2, glm::vec3 scale1, glm::vec3 scale2,
		bool wall);

	std::vector<CollisionAgent*> agents;
	std::vector<CollisionAgent*> walls;
	std::vector<CollisionAgent*> removedAgents;

	CollisionAgent* registerAgent(Scene::Transform* transform, bool wall);
	CollisionAgent* registerAgent(Scene::Transform* positionTransform, Scene::Transform* collisionTransform, bool wall);

	bool checkCollision(CollisionAgent* A, CollisionAgent* B);

	void unregisterAgent(CollisionAgent* agent, bool wall);

	void update();
};