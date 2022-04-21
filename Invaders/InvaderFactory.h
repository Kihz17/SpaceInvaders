#pragma once

#include "InvaderObject.h"

#include <glm/glm.hpp>

#include <string>

class InvaderFactory
{
public:
	static InvaderObject* MakeObject(const std::string& type, const glm::vec3& position);
};