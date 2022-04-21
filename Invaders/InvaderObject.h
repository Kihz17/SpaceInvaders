#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

class InvaderObjectImpl;
class InvaderObject
{
public:
	InvaderObject(const glm::vec3& position, std::vector<std::string> renderHandles, const std::string& deathPose);
	virtual ~InvaderObject();

	virtual void OnMove() const;
	virtual void OnDeath() const;
	virtual std::string GetRenderHandle() const;

	virtual glm::vec3& GetPosition();
	virtual const glm::vec3& GetPositionSafe() const;

	bool IsDead() const;

protected:
	InvaderObjectImpl* impl;
};