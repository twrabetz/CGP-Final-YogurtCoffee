#include "CollisionManager.hpp"

//Handle collisions:
bool CollisionManager::checkCollision(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 rad1, glm::vec3 rad2, glm::vec3 scale1, glm::vec3 scale2, 
	CollisionAxis& outAxis /*=0*/)
{
	float xDiff = abs(pos1.x - pos2.x);
	float yDiff = abs(pos1.y - pos2.y);
	float zDiff = abs(pos1.z - pos2.z);
	if (zDiff > xDiff && zDiff > yDiff)
		outAxis = CollisionAxis::Z;
	else if (yDiff > xDiff)
		outAxis = CollisionAxis::Y;
	else
		outAxis = CollisionAxis::X;
	bool result = xDiff <= rad1.x * scale1.x + rad2.x * scale2.x && yDiff <= rad1.y * scale1.y + rad2.y * scale2.y 
		&& zDiff <= rad1.z * scale1.z + rad2.z * scale2.z;
	return result;
}

bool CollisionManager::checkCollision(CollisionAgent* A, CollisionAgent* B, CollisionAxis& outAxis /*=0*/)
{
	return checkCollision(A->positionTransform->position, B->positionTransform->position, A->scaleTransform->radius, B->scaleTransform->radius,
		A->scaleTransform->scale, B->scaleTransform->scale, outAxis);
}

float CollisionManager::combineFloat(float a, float b)
{
	if (a == 0)
		return b;
	if (b == 0)
		return a;
	if ((a < 0 && b >= 0) || (a >= 0 && b < 0))
	{
		return a + b;
	}
	float res = (a / abs(a)) * std::max<float>(abs(a), abs(b));
	return res;
}

glm::vec3 CollisionManager::combine(glm::vec3 offset1, glm::vec3 offset2)
{
	return glm::vec3(combineFloat(offset1.x, offset2.x), combineFloat(offset1.y, offset2.y), combineFloat(offset1.z, offset2.z));
}

//Make everything go ahead n bump everything else
void CollisionManager::processCollision(CollisionAgent* agent, glm::vec3 pos1, glm::vec3 pos2, glm::vec3 rad1, glm::vec3 rad2, glm::vec3 scale1,
	glm::vec3 scale2, bool wall)
{
	float xDiff = abs(pos1.x - pos2.x);
	float yDiff = abs(pos1.y - pos2.y);
	float zDiff = abs(pos1.z - pos2.z);
	float xPush = std::max<float>(rad1.x * scale1.x + rad2.x * scale2.x - abs(xDiff), 0);
	float yPush = std::max<float>(rad1.y * scale1.y + rad2.y * scale2.y - abs(yDiff), 0);
	float zPush = std::max<float>(rad1.z * scale1.z + rad2.z * scale2.z - abs(zDiff), 0);
	if (zDiff > xDiff && zDiff > yDiff)
	{
		agent->collidedPrevFrame = true;
		agent->velocity.z = pos1.z >= pos2.z ? std::max<float>(0.0f, agent->velocity.z) : std::min<float>(0.0f, agent->velocity.z);
		agent->offset = combine(agent->offset, glm::vec3(0.0f, 0.0f, pos1.z >= pos2.z ? (wall ? zPush : zPush / 2.0f) : (wall ? -zPush : -zPush / 2.0f)));
	}
	else if (xDiff > yDiff)
		agent->offset = combine(agent->offset, glm::vec3(pos1.x >= pos2.x ? (wall ? xPush : xPush / 2.0f) : (wall ? -xPush : -xPush / 2.0f), 0.0f, 0.0f));
	else
		agent->offset = combine(agent->offset, glm::vec3(0.0f, pos1.y >= pos2.y ? (wall ? yPush : yPush / 2.0f) : (wall ? -yPush : -yPush / 2.0f), 0.0f));
}

CollisionAgent* CollisionManager::registerAgent(Scene::Transform* transform, bool wall)
{
	CollisionAgent* newAgent = new CollisionAgent(transform);
	(wall ? walls : agents).push_back(newAgent);
	return newAgent;
}

CollisionAgent* CollisionManager::registerAgent(Scene::Transform* transform, Scene::Transform* scaleTransform, bool wall)
{
	CollisionAgent* newAgent = new CollisionAgent(transform, scaleTransform);
	(wall ? walls : agents).push_back(newAgent);
	return newAgent;
}

void CollisionManager::unregisterAgent(CollisionAgent* agent, bool wall)
{
	bool erased = false;
	for (int i = 0; i < (wall ? walls : agents).size(); i++)
	{
		if (agent == (wall ? walls : agents)[i])
		{
			(wall ? walls : agents).erase((wall ? walls : agents).begin() + i);
			erased = true;
			break;
		}
	}
	if (erased)
	{
		removedAgents.push_back(agent);
	}
}

void CollisionManager::update()
{

	for (CollisionAgent* agent : agents)
	{
		for (CollisionAgent* agent2 : agents)
		{
			if (agent != agent2 && agent->enabled && agent2->enabled && 
				checkCollision(agent->positionTransform->position, agent2->positionTransform->position, agent->scaleTransform->radius, 
					agent2->scaleTransform->radius, agent->scaleTransform->scale, agent2->scaleTransform->scale))
				processCollision(agent, agent->positionTransform->position, agent2->positionTransform->position, agent->scaleTransform->radius, 
					agent2->scaleTransform->radius, agent->scaleTransform->scale, agent2->scaleTransform->scale, false);
		}
	}

	for (CollisionAgent* agent : agents)
	{
		agent->positionTransform->position += agent->offset;
		agent->offset = glm::vec3(0.0f);
	}

	for (CollisionAgent* agent : agents)
	{
		for (CollisionAgent* wall : walls)
		{
			if (agent != wall && agent->enabled && wall->enabled && 
				checkCollision(agent->positionTransform->position, wall->positionTransform->position,
				agent->scaleTransform->radius, wall->scaleTransform->radius, agent->scaleTransform->scale, wall->scaleTransform->scale))
				processCollision(agent, agent->positionTransform->position, wall->positionTransform->position, agent->scaleTransform->radius,
					wall->scaleTransform->radius, agent->scaleTransform->scale, wall->scaleTransform->scale, true);
		}
	}

	for (CollisionAgent* agent : agents)
	{
		agent->positionTransform->position += agent->offset;
		agent->offset = glm::vec3(0.0f);
	}
}

CollisionManager::~CollisionManager()
{
	for (int i = 0; i < agents.size(); i++)
	{
		free(agents[i]);
	}
	for (int i = 0; i < walls.size(); i++)
	{
		free(walls[i]);
	}
	for (int i = 0; i < removedAgents.size(); i++)
	{
		free(removedAgents[i]);
	}
}