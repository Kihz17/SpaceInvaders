#include "InvaderObject.h"
#include "InvaderObjectImpl.h"

InvaderObject::InvaderObject(const glm::vec3& position, std::vector<std::string> renderHandles, const std::string& deathPose)
	: impl(new InvaderObjectImpl(position, renderHandles, deathPose))
{

}

InvaderObject::~InvaderObject()
{
	delete impl;
}

glm::vec3& InvaderObject::GetPosition()
{
	return impl->position;
}

const glm::vec3& InvaderObject::GetPositionSafe() const
{
	return impl->position;
}

void InvaderObject::OnMove() const
{
	if(impl->currentRenderHandle + 1 >= impl->renderHandles.size())
	{
		impl->currentRenderHandle = 0;
	}
	else
	{
		impl->currentRenderHandle++;
	}
}

void InvaderObject::OnDeath() const
{
	impl->dead = true;
}

std::string InvaderObject::GetRenderHandle() const
{
	if (impl->dead)
	{
		return impl->deathPoseHandle;
	}

	return impl->renderHandles[impl->currentRenderHandle];
}

bool InvaderObject::IsDead() const
{
	return impl->dead;
}