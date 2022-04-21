#pragma once

#include "InvaderStructureManager.h"

#include <glm/glm.hpp>

class InvaderShield
{
public:
	InvaderShield(const glm::vec3& position);
	virtual ~InvaderShield();

	void DestroyPixel(int index);
	void Render(Ref<Shader> shader) const;
	bool IsCollision(const glm::vec3& position, int& collidedIndex);

	inline const glm::vec3& GetPositionSafe() const { return position; }
	inline glm::vec3& GetPosition() { return position; }

private:
	glm::vec3 position;
	InvaderStructure shieldStructure;
};