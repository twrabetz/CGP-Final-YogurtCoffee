#include "DrunkPerson.hpp"
#include <iostream>

DrunkPerson::DrunkPerson(CollisionAgent* newAgent, Scene::Transform* newTarget)
{
	agent = newAgent;
	target = newTarget;
}

void DrunkPerson::update(float elapsed, float gravity)
{
	if (agent->enabled)
	{
		agent->velocity -= glm::vec3(0, 0, 1.0f) * gravity * elapsed;
		agent->positionTransform->position += agent->velocity * elapsed;
	}
	else
	{
		//Lerp to target
		agent->velocity = glm::vec3(0.0f);
		glm::vec3 diff = target->position + pickupOffset - agent->positionTransform->position;
		agent->positionTransform->position += glm::normalize(diff) * std::min<float>(glm::length(diff), lerpSpeed);
	}

	agent->velocity *= std::pow(0.15f, elapsed);

	agent->collidedPrevFrame = false;
}

void DrunkPerson::pickUp()
{
	agent->enabled = false;
}

void DrunkPerson::launch(glm::vec3 velocity)
{
	agent->velocity = velocity;
	agent->enabled = true;
}