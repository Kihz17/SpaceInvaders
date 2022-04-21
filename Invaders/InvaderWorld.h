#pragma once 

#include "InvaderObject.h"
#include "AABB.h"
#include "IMessage.h"
#include "PlayerController.h"
#include "InvaderShield.h"

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

class InvaderWorld : public IMessage
{
public:
	InvaderWorld();
	virtual ~InvaderWorld();

	void Initialize();
	void OnUpdate(float deltaTime);
	void CleanUp();

	inline void SetPlayer(PlayerController* player) 
	{
		this->player = player;
		player->SetReciever(this);
	}

	std::vector<const InvaderObject*> GetInvaderObjects() const;
	std::vector<const InvaderShield*> GetShields() const;

	const AABB& GetWorldBounds() const { return worldBounds; }

	glm::vec3 GetPlayerSpawnPoint() const;

	inline int GetScore() const { return score; }

	virtual bool RecieveMessage(Message theMessage) override;
	virtual bool SetReciever(IMessage* pTheReciever) override;


	static const float EnemyOffset;
	static const float LoseYThreshold;

private:
	void KillEnemy(InvaderObject* enemy);
	void TryChangeMoveDirection();
	float GetEnemyMoveWaitTime() const;

	std::vector<std::vector<InvaderObject*>> enemies;
	std::unordered_map<InvaderObject*, float> enemyDeathTimers;

	std::vector<InvaderObject*> bombs;

	std::vector<InvaderShield*> shields;

	float lastMoveTime;
	float lastWaveTime;
	const float waveTime;
	glm::vec3 moveDirection;
	int moveIndex;
	float currentBombMoveTime;

	const float minEdge;
	const float maxEdge;

	AABB worldBounds;

	bool lost;

	InvaderObject* ufo;
	float currentUFOTime;
	float UFOTime;
	glm::vec3 ufoVelocity;

	PlayerController* player;
	InvaderObject* playerBullet;
	float playerDeathTime;
	float playerBulletDeathTime;

	int score;
};