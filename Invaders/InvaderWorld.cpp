#include "InvaderWorld.h"
#include "InvaderStructureManager.h"
#include "InvaderFactory.h"

const float InvaderWorld::EnemyOffset = InvaderStructureManager::GetInvaderCubeSize() * 14.0f;
const float InvaderWorld::LoseYThreshold = EnemyOffset * -5.0f;

static const int MaxEnemyRows = 5;
static const int MaxEnemyWidth = 11;
static const glm::vec3 enemyStartPosition(0.0f, 0.0f, 0.0f);
static const float BulletYThreshold = InvaderWorld::EnemyOffset * -10.0f;
static const float ArenaWidthScale = 3.0f;
static const float DownForce = InvaderStructureManager::GetInvaderCubeSize() * 4.0f;
static const glm::vec3 PlayerBulletTravelSpeed(0.0f, 1000.0f, 0.0f);
static const glm::vec3 BombTravelSpeed(0.0f, -900.0f, 0.0f);
static const float BombMoveTime = 0.05f;
static const float PlayerBulletDeathTime = 0.3f;
static const float PlayerDeathTime = 0.3f;
static const float EnemyDeathTime = 0.3f;
static const float UFOMoveSpeed = 180.0f;

static const int ShieldCount = 4;
static const float ShieldOffset = 500.0f;

static float GetRandomFloat(float low, float high)
{
	return low + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (high - low));
}

InvaderWorld::InvaderWorld()
	: lastMoveTime(0.0f),
	moveDirection(0.0f, 0.0f, 30.0f),
	moveIndex(0),
	minEdge(enemyStartPosition.z - glm::length(moveDirection) * ArenaWidthScale),
	maxEdge(enemyStartPosition.z + (MaxEnemyWidth * EnemyOffset) + glm::length(moveDirection) * ArenaWidthScale),
	waveTime(0.05f),
	lastWaveTime(waveTime),
	worldBounds(glm::vec3(0.0f, LoseYThreshold, minEdge), glm::vec3(0.0f, (enemyStartPosition.y + MaxEnemyRows * EnemyOffset) - 20.0f, maxEdge)),
	lost(true),
	player(nullptr),
	playerBullet(nullptr),
	playerBulletDeathTime(0.0f),
	currentUFOTime(0.0f),
	UFOTime(GetRandomFloat(10.0f, 50.0f)),
	ufo(nullptr),
	ufoVelocity(0.0f),
	currentBombMoveTime(0.0f),
	score(0)
{

}

InvaderWorld::~InvaderWorld()
{
	CleanUp();
}

void InvaderWorld::Initialize()
{
	CleanUp();

	score = 0;

	glm::vec3 currentPos = enemyStartPosition;
	for (int i = 0; i < MaxEnemyRows; i++)
	{
		std::vector<InvaderObject*> enemyVec;
		std::string enemyType = i <= 1 ? "I3" : i <= 3 ? "I1" : "I2";

		enemyVec.resize(MaxEnemyWidth);
		for (int j = 0; j < MaxEnemyWidth; j++)
		{
			enemyVec[j] = InvaderFactory::MakeObject(enemyType, currentPos);
			currentPos.z += EnemyOffset;
		}

		currentPos.z = enemyStartPosition.z;
		currentPos.y += EnemyOffset;

		enemies.push_back(enemyVec);
	}

	// Setup shields
	glm::vec3 shieldStartPos(0.0f, LoseYThreshold, minEdge + 40.0f);
	for(int i = 0; i < ShieldCount; i++)
	{
		shields.push_back(new InvaderShield(shieldStartPos));
		shieldStartPos.z += ShieldOffset;
	}

	player->Reset(GetPlayerSpawnPoint());
}

void InvaderWorld::OnUpdate(float deltaTime)
{
	if (player->IsDead())
	{
		if (this->playerDeathTime >= PlayerDeathTime)
		{
			lost = true;
		}
		else
		{
			playerDeathTime += PlayerDeathTime;
		}
	}

	// Play enemy death pose
	for (int i = 0; i < enemies.size(); i++)
	{
		std::vector<InvaderObject*>& batch = enemies[i];
		std::vector<InvaderObject*>::iterator it = batch.begin();
		while (it != batch.end())
		{
			bool incIt = true;
			InvaderObject* enemy = (*it);

			if (enemy->IsDead()) // We are dead, incremet our death timers
			{
				float& timer = enemyDeathTimers.at(enemy);
				if (timer >= EnemyDeathTime) // Death timer expired, remove the enemy
				{
					it = batch.erase(it);
					enemyDeathTimers.erase(enemy);
					delete enemy;
					incIt = false;
				}
			
				timer += deltaTime;
			}

			if(incIt) it++;
		}
	}

	// Update enemy movement
	if (!lost)
	{
		lastMoveTime += deltaTime;
		if (lastMoveTime >= GetEnemyMoveWaitTime()) // Time to move the enemies
		{
			lastWaveTime += deltaTime;
			if (lastWaveTime >= waveTime)
			{
				lastWaveTime = 0.0f;
				std::vector<InvaderObject*>& batch = enemies[moveIndex++];
				for (int i = 0; i < batch.size(); i++) // Move the current "batch of enemies
				{
					if (batch[i]->IsDead()) continue; // We are dead, don't move

					batch[i]->GetPosition() += moveDirection;
					batch[i]->OnMove();
				}
			}

			if (moveIndex >= enemies.size()) // We have moved all the enemies, start counting again
			{
				lastMoveTime = 0.0f;
				moveIndex = 0;
				lastWaveTime = waveTime;
				TryChangeMoveDirection();
			}
		}

		// Drop bombs
		{
			for (int i = 0; i < enemies.size(); i++)
			{
				std::vector<InvaderObject*>& batch = enemies[i];
				for (int j = 0; j < batch.size(); j++)
				{
					if (GetRandomFloat(0.0f, 1.0f) <= 0.0008f) // .008% chance to drop a bomb
					{
						InvaderObject* enemy = batch[j];
						const glm::vec3& pos = enemy->GetPosition();
						float cubeSize = InvaderStructureManager::GetInvaderCubeSize();
						float width = InvaderStructureManager::GetStructure(enemy->GetRenderHandle())->width;
						float height = InvaderStructureManager::GetStructure(enemy->GetRenderHandle())->height;
						bombs.push_back(InvaderFactory::MakeObject("Bomb", glm::vec3(0.0f, pos.y - cubeSize * height, pos.z + (cubeSize * width) / 2.0f)));
					}
				}
			}
		}

		if (ufo) // UFO is spawned, move it
		{
			glm::vec3& ufoPos = ufo->GetPosition();
			if (ufoPos.z <= minEdge || ufoPos.z >= maxEdge) // Out of bounds, remove
			{
				delete ufo;
				ufo = nullptr;

				// Generate a new spawn time
				UFOTime = GetRandomFloat(10.0f, 50.0f);
				currentUFOTime = 0.0f;
			}
			else // In bounds, move
			{
				ufoPos += ufoVelocity * deltaTime;
			}
		}
		else
		{
			if (currentUFOTime >= UFOTime)
			{
				// Spawn UFO
				bool left = GetRandomFloat(0.0f, 10.0f) <= 5.0f ? true : false;
				ufo = InvaderFactory::MakeObject("UFO", glm::vec3(0.0f, worldBounds.max.y, left ? minEdge + 1.0f : maxEdge - 1.0f));

				float ufoMoveSpeed = left ? UFOMoveSpeed : -UFOMoveSpeed;
				ufoVelocity = glm::vec3(0.0f, 0.0f, ufoMoveSpeed);
			}
			else
			{
				currentUFOTime += deltaTime;
			}
		}
	}	

	// Update bomb movement
	if(currentBombMoveTime >= BombMoveTime)
	{
		currentBombMoveTime = 0.0f;
		std::vector<InvaderObject*>::iterator it = bombs.begin();
		while (it != bombs.end())
		{
			InvaderObject* bomb = (*it);
			glm::vec3& pos = bomb->GetPosition();

			if (pos.y <= BulletYThreshold) // Out of bounds, remove
			{
				it = bombs.erase(it);
				delete bomb;
			}
			else
			{
				bool incIt = true;

				pos += BombTravelSpeed * deltaTime;
				bomb->OnMove();
				
				float cubeSize = InvaderStructureManager::GetInvaderCubeSize();
				float bombHeight = InvaderStructureManager::GetStructure(bomb->GetRenderHandle())->height;

				// Bombs are 1 in width, so we don't have to worry about horizontal
				bool playerCollision = InvaderStructureManager::IsCollision(player->GetRenderHandle(), player->GetPosition(), glm::vec3(pos.x, pos.y - (cubeSize * bombHeight), pos.z));
				if (playerCollision) // We collided!
				{ 
					player->OnDeath();
					it = bombs.erase(it);
					delete bomb;
					incIt = false;
				}

				if (!playerCollision) // We didn't collide with the player, check shields
				{
					for (InvaderShield* shield : shields)
					{
						int hitIndex = -1;

						if (shield->IsCollision(glm::vec3(pos.x, pos.y - (cubeSize * bombHeight), pos.z), hitIndex))  // We hit a shield!
						{
							shield->DestroyPixel(hitIndex);
							it = bombs.erase(it);
							delete bomb;
							incIt = false;
							break;
						}
					}
				}

				if(incIt) it++;
			}
		}
	}
	currentBombMoveTime += deltaTime;

	// Update bullet movement and hit detection
	if (playerBullet)
	{
		if (playerBullet->IsDead()) // Bullet is dead, increment death timer
		{
			if (this->playerBulletDeathTime >= PlayerBulletDeathTime)
			{
				delete playerBullet;
				playerBullet = nullptr;
			}
			else
			{
				this->playerBulletDeathTime += deltaTime;
			}
		}
		else
		{
			glm::vec3& bulletPos = playerBullet->GetPosition();
			if (bulletPos.y >= worldBounds.max.y) // We are out of bounds, kill the bullet
			{
				playerBullet->OnDeath();
				this->playerBulletDeathTime = 0.0f;
			}
			else // We are in the playing field, move
			{
				bulletPos += PlayerBulletTravelSpeed * deltaTime; 
				playerBullet->OnMove();

				// Check collision
				for (std::vector<InvaderObject*>& row : this->enemies)
				{
					for (InvaderObject* enemy : row)
					{
						if (enemy->IsDead()) continue;

						if (InvaderStructureManager::IsCollision(enemy->GetRenderHandle(), enemy->GetPosition(), bulletPos)) // We collided!
						{ 
							KillEnemy(enemy);
							delete playerBullet;
							playerBullet = nullptr;
							score += 50;
							break;
						}
					}
				}
			}	
		}		
	}
}

void InvaderWorld::TryChangeMoveDirection()
{
	bool needsChange = false;
	for (int i = 0; i < enemies.size(); i++)
	{
		std::vector<InvaderObject*>& batch = enemies[i];

		// Get the left most enemy in this row
		InvaderObject* leftEnemy = nullptr;
		for (int j = 0; j < batch.size(); j++)
		{
			if (!batch[j]->IsDead())
			{
				leftEnemy = batch[j];
				break;
			}
		}

		// Get the right most enemy in this row
		InvaderObject* rightEnemy = nullptr;
		for (int j = batch.size() - 1; j >= 0; j--)
		{
			if (!batch[j]->IsDead())
			{
				rightEnemy = batch[j];
				break;
			}
		}

		// Are we within bounds? If not change directions
		if (leftEnemy)
		{
			const glm::vec3& leftPos = leftEnemy->GetPosition();
			if (leftPos.z <= minEdge || leftPos.z >= maxEdge)
			{
				needsChange = true;
				moveDirection *= -1.0f;
				break;
			}
		}
	
		if (rightEnemy)
		{
			const glm::vec3& rightPos = rightEnemy->GetPosition();
			if (rightPos.z <= minEdge || rightPos.z >= maxEdge)
			{
				needsChange = true;
				moveDirection *= -1.0f;
				break;
			}
		}	
	}

	if (needsChange) // We have changed directions, move us down
	{
		for (int i = 0; i < enemies.size(); i++)
		{
			std::vector<InvaderObject*>& batch = enemies[i];
			for (int j = 0; j < batch.size(); j++)
			{
				InvaderObject* enemy = batch[j];
				enemy->GetPosition().y -= DownForce;
				if (enemy->GetPosition().y - (EnemyOffset - (InvaderStructureManager::GetInvaderCubeSize() * 8.0f)) <= LoseYThreshold) // We lost
				{
					lost = true;
				}
			}
		}
	}
}

void InvaderWorld::CleanUp()
{
	for (std::vector<InvaderObject*>& row : enemies)
	{
		for (InvaderObject* enemy : row)
		{
			delete enemy;
		}
	}

	enemies.clear();
	enemyDeathTimers.clear();

	for (InvaderShield* shield : shields)
	{
		delete shield;
	}

	shields.clear();

	if (playerBullet)
	{
		delete playerBullet;
		playerBullet = nullptr;
	}	

	playerBulletDeathTime = 0.0f;

	if (ufo)
	{
		delete ufo;
		ufo = nullptr;
	}

	currentUFOTime = 0.0f;
}

std::vector<const InvaderObject*> InvaderWorld::GetInvaderObjects() const
{
	std::vector<const InvaderObject*> objects;
	for (std::vector<InvaderObject*> row : this->enemies)
	{
		for (InvaderObject* enemy : row)
		{
			objects.push_back(enemy);
		}
	}

	for (InvaderObject* bomb : bombs)
	{
		objects.push_back(bomb);
	}

	if (playerBullet)
	{
		objects.push_back(playerBullet);
	}

	if (ufo)
	{
		objects.push_back(ufo);
	}

	return objects;
}

glm::vec3 InvaderWorld::GetPlayerSpawnPoint() const
{
	return glm::vec3(0.0f, LoseYThreshold - 160.0f, (minEdge + maxEdge) / 2.0f);
}

bool InvaderWorld::RecieveMessage(Message message)
{
	const std::string& command = message.command;
	if (command == "PlayerMove")
	{
		if (player->IsDead() || lost) return true;
		glm::vec3 moveDirection = glm::vec3(message.fData[0]);
		glm::vec3& position = player->GetPosition();

		// Adjust the player move if we have to
		glm::vec3 positionAfterMove = position + moveDirection;
		if (positionAfterMove.z < minEdge)
		{
			positionAfterMove.z = minEdge;
		}
		else if (positionAfterMove.z > maxEdge)
		{
			positionAfterMove.z = maxEdge;
		}

		// Move the player
		position = positionAfterMove;
		return true;
	}
	else if (command == "PlayerShoot")
	{
		if (player->IsDead() || lost) return true;
		if (playerBullet) return true; // Bullet is still travelling, can't shoot

		const glm::vec3& playerPos = player->GetPosition();
		playerBullet = InvaderFactory::MakeObject("PlayerBullet", glm::vec3(0.0f, playerPos.y, playerPos.z + 
			((InvaderStructureManager::GetStructure(player->GetRenderHandle())->width * InvaderStructureManager::GetInvaderCubeSize()) / 2.0f)));

		return true;
	}
	else if (command == "StartGame")
	{
		if (!lost) return true; // Game is still happening
		Initialize();
		lost = false;
	}

	return false;
}

bool InvaderWorld::SetReciever(IMessage* pTheReciever)
{
	return false;
}

void InvaderWorld::KillEnemy(InvaderObject* enemy)
{
	enemy->OnDeath();
	enemyDeathTimers.insert({ enemy, 0.0f });
}

float InvaderWorld::GetEnemyMoveWaitTime() const
{
	int enemyCount = 0;
	for (const std::vector<InvaderObject*>& vec : enemies) enemyCount += vec.size();
	return (float)enemyCount / 180.0f;
}

std::vector<const InvaderShield*> InvaderWorld::GetShields() const
{
	std::vector<const InvaderShield*> vec;
	for (InvaderShield* shield : shields) vec.push_back(shield);
	return vec;
}