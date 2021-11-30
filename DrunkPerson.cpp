#include "DrunkPerson.hpp"
#include <iostream>

#include <glm/gtx/string_cast.hpp>

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
		glm::vec3 diff = target->get_global_position() + pickupOffset - agent->GetPosition();
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
	//agent->positionTransform->position = glm::vec3(0.004437f, agent->positionTransform->position.y, agent->positionTransform->position.z);
	agent->positionTransform->position += glm::vec3(0, 0, 3.0f);
	agent->velocity = velocity;
	agent->enabled = true;
	//std::cout << toString(agent->positionTransform->position) << std::endl;
}

void DrunkPerson::inTrashBin(Scene::Transform* trashBin)
{
	agent->enabled = false;
	target = trashBin;
}