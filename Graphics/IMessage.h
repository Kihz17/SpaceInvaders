#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

struct Message
{
	std::string command;
	std::vector<std::string> sData;
	std::vector<glm::vec4> fData;
};

class IMessage
{
public:
	virtual ~IMessage() = default;

	virtual bool RecieveMessage(Message theMessage) = 0;
	virtual bool SetReciever(IMessage* pTheReciever) = 0;
};