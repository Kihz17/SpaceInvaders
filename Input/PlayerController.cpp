#include "PlayerController.h"
#include "Input.h"
#include "InvaderObjectImpl.h"

static const glm::vec3 PlayerMoveDirection(0.0f, 0.0f, 600.0f);

PlayerController::PlayerController(const glm::vec3& position)
	: InvaderObject(position, {"player"}, "playerExplosion")
{

}

void PlayerController::OnUpdate(float deltaTime)
{
	if (receiver)
	{
		if (Input::IsKeyPressed(Key::Left))
		{
			glm::vec3 velocity = -PlayerMoveDirection * deltaTime;

			Message message;
			message.fData.push_back(glm::vec4(velocity, 1.0f));
			message.command = "PlayerMove";
			receiver->RecieveMessage(message);
		}
		else if (Input::IsKeyPressed(Key::Right))
		{
			glm::vec3 velocity = PlayerMoveDirection * deltaTime;

			Message message;
			message.fData.push_back(glm::vec4(velocity, 1.0f));
			message.command = "PlayerMove";
			receiver->RecieveMessage(message);
		}

		if (Input::IsKeyPressed(Key::Space))
		{
			Message message;
			message.command = "PlayerShoot";
			receiver->RecieveMessage(message);
		}

		if (Input::IsKeyPressed(Key::Enter))
		{
			Message message;
			message.command = "StartGame";
			receiver->RecieveMessage(message);
		}
	}
}

bool PlayerController::RecieveMessage(Message theMessage)
{
	return false;
}

void PlayerController::Reset(const glm::vec3& position)
{
	this->impl->dead = false;
	this->impl->currentRenderHandle = 0;
	this->impl->position = position;
}