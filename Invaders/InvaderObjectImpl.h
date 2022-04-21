#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <string>
struct InvaderObjectImpl
{
	InvaderObjectImpl(const glm::vec3& positon, std::vector<std::string> renderHandles, const std::string& deathPose)
		: position(positon), 
		renderHandles(renderHandles),
		currentRenderHandle(0),
		deathPoseHandle(deathPose),
		dead(false) {}

	~InvaderObjectImpl() = default;

	glm::vec3 position;
	std::vector<std::string> renderHandles;
	std::string deathPoseHandle;
	unsigned int currentRenderHandle;
	bool dead;
};