#pragma once

#include "InvaderObject.h"
#include "IMessage.h"

#include <glm/glm.hpp>
#include <string>

class PlayerController : public InvaderObject, public IMessage
{
public:
	PlayerController(const glm::vec3& position);
	~PlayerController() = default;

	void OnUpdate(float deltaTime);
	void Reset(const glm::vec3& postition);

	virtual bool RecieveMessage(Message theMessage) override;
	inline virtual bool SetReciever(IMessage* pTheReciever) override
	{
		receiver = pTheReciever;
		return true;
	}

private:
	IMessage* receiver;
};