#include "InvaderFactory.h"
#include <stdio.h>

InvaderObject* InvaderFactory::MakeObject(const std::string& type, const glm::vec3& position)
{
	if (type == "I1")
	{
		return new InvaderObject(position, { "I11", "I12" }, "explosion1");
	}
	else if (type == "I2")
	{
		return new InvaderObject(position, { "I21", "I22" }, "explosion1");
	}
	else if (type == "I3")
	{
		return new InvaderObject(position, { "I31", "I32" }, "explosion1");
	}
	else if (type == "PlayerBullet")
	{
		return new InvaderObject(position, { "playerBullet" }, "explosion2");
	}
	else if (type == "UFO")
	{
		return new InvaderObject(position, { "ufo" }, "explosion1");
	}
	else if (type == "Bomb")
	{
		return new InvaderObject(position, { "bomb1", "bomb2", "bomb3"}, "");
	}

	printf("Could not make invader object %s\n", type);
	return nullptr;
}