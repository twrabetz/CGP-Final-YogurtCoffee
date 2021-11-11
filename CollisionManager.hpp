#pragma once

#include <glm/glm.hpp>
#include "Scene.hpp"

enum CollisionAxis
{
	X,
	Y,
	Z
};

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

	glm::vec3 GetPosition()
	{
		return positionTransform->make_local_to_world() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
};

struct CollisionManager
{

	~CollisionManager();

	//Handle collisions:
	inline bool checkCollision(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 rad1, glm::vec3 rad2, glm::vec3 scale1, glm::vec3 scale2)
	{
		CollisionAxis dummy = CollisionAxis::X;
		return checkCollision(pos1, pos2, rad1, rad2, scale1, scale2, dummy);
	}
	bool checkCollision(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 rad1, glm::vec3 rad2, glm::vec3 scale1, glm::vec3 scale2, 
		CollisionAxis& outAxis);

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

	inline bool checkCollision(CollisionAgent* A, CollisionAgent* B) 
	{
		CollisionAxis dummy = CollisionAxis::X;
		return checkCollision(A, B, dummy); 
	}
	bool checkCollision(CollisionAgent* A, CollisionAgent* B, CollisionAxis& outAxis);

	void unregisterAgent(CollisionAgent* agent, bool wall);

	void update();
};