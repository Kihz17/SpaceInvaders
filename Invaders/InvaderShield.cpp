#include "InvaderShield.h"
#include "Renderer.h"

InvaderShield::InvaderShield(const glm::vec3& position)
	: shieldStructure(*InvaderStructureManager::GetStructure("shield")), position(position)
{
}

InvaderShield::~InvaderShield()
{

}

void InvaderShield::DestroyPixel(int index)
{
	int size = shieldStructure.GetSize();
	if (index < 0 && index >= size) return;

	shieldStructure.structure[index] = false;
}

void InvaderShield::Render(Ref<Shader> shader) const
{
	float cubeSize = InvaderStructureManager::GetInvaderCubeSize();

	glm::mat4 cubeTransform(1.0f);
	cubeTransform[3] = glm::vec4(position, 1.0f);
	for (int i = 0; i < shieldStructure.GetSize(); i++)
	{
		if (shieldStructure.structure[i])
		{
			glm::mat4 transform = cubeTransform;
			Renderer::RenderMeshWithColorOverride(shader, InvaderStructureManager::GetCubeMesh(), transform, shieldStructure.color, false, true);
		}

		if ((i + 1) % shieldStructure.width == 0) // Move down
		{

			cubeTransform[3].z = position.z;
			cubeTransform[3].y -= cubeSize;
		}
		else
		{
			cubeTransform[3].z += cubeSize;
		}
	}
}

bool InvaderShield::IsCollision(const glm::vec3& position, int& collidedIndex)
{
	glm::mat4 transform(1.0f);
	transform[3] = glm::vec4(this->position, 1.0f);

	for (int i = 0; i < shieldStructure.GetSize(); i++)
	{
		if (shieldStructure.structure[i]) // There is a cube mesh here
		{
			if (InvaderStructureManager::GetCubeMesh()->GetBoundingBox().IsInside(transform, position))
			{
				collidedIndex = i;
				return true; // We have collided with one of the cubes
			}
		}

		if ((i + 1) % shieldStructure.width == 0) // Move down
		{

			transform[3].z = this->position.z;
			transform[3].y -= InvaderStructureManager::GetInvaderCubeSize();
		}
		else
		{
			transform[3].z += InvaderStructureManager::GetInvaderCubeSize();
		}
	}

	collidedIndex = -1;
	return false;
}