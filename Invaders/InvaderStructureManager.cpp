#include "InvaderStructureManager.h"
#include "Renderer.h"

#include <sstream>

Ref<Mesh> InvaderStructureManager::invaderCube = nullptr;
std::unordered_map<std::string, InvaderStructure*> InvaderStructureManager::structures;

const float InvaderStructureManager::InvaderCubeSize = 10.0f;

void InvaderStructureManager::Initialize()
{
	// Load cube mesh
	{
		std::stringstream ss;
		ss << SOLUTION_DIR << "Extern\\assets\\models\\Invader_Single_Cube.ply";
		InvaderStructureManager::invaderCube = CreateRef<Mesh>(ss.str());
	}
	
	std::string invaderDataFolder;
	{
		std::stringstream ss;
		ss << SOLUTION_DIR << "Extern\\assets\\invader_data\\";
		invaderDataFolder = ss.str();
	}

	// Base
	{
		std::string path(invaderDataFolder + "SpaceInvader_Base_block.txt");
		structures.insert({ "player", new InvaderStructure(path, glm::vec4(0.0f, 0.8f, 0.0f, 1.0f))});
	}
	{
		std::string path(invaderDataFolder + "SpaceInvader_Player_Bullet.txt");
		structures.insert({ "playerBullet", new InvaderStructure(path, glm::vec4(0.0f, 0.0f, 0.8f, 1.0f)) });
	}
	{
		std::string path(invaderDataFolder + "SpaceInvader_Player_Explosion.txt");
		structures.insert({ "playerExplosion", new InvaderStructure(path, glm::vec4(0.0f, 0.8f, 0.0f, 1.0f)) });
	}

	// Shield
	{
		std::string path(invaderDataFolder + "SpaceInvader_Shield_block.txt");
		structures.insert({ "shield", new InvaderStructure(path, glm::vec4(0.0f, 0.8f, 0.0f, 1.0f)) });
	}

	// Bombs
	{
		std::string path(invaderDataFolder + "SpaceInvader_Bomb_pose_1.txt");
		structures.insert({ "bomb1", new InvaderStructure(path, glm::vec4(0.8f, 0.0f, 0.0f, 1.0f)) });
	}
	{
		std::string path(invaderDataFolder + "SpaceInvader_Bomb_pose_2.txt");
		structures.insert({ "bomb2", new InvaderStructure(path, glm::vec4(0.8f, 0.0f, 0.0f, 1.0f)) });
	}
	{
		std::string path(invaderDataFolder + "SpaceInvader_Bomb_pose_3.txt");
		structures.insert({ "bomb3", new InvaderStructure(path, glm::vec4(0.8f, 0.0f, 0.0f, 1.0f)) });
	}

	// Explosions
	{
		std::string path(invaderDataFolder + "SpaceInvader_Explosion_1.txt");
		structures.insert({ "explosion1", new InvaderStructure(path, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)) });
	}
	{
		std::string path(invaderDataFolder + "SpaceInvader_Explosion_2.txt");
		structures.insert({ "explosion2", new InvaderStructure(path, glm::vec4(0.8f, 0.0f, 0.0f, 1.0f)) });
	}

	// Enemies
	{
		std::string path(invaderDataFolder + "SpaceInvader_I1_block_pose_1.txt");
		structures.insert({ "I11", new InvaderStructure(path, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)) });
	}
	{
		std::string path(invaderDataFolder + "SpaceInvader_I1_block_pose_2.txt");
		structures.insert({ "I12", new InvaderStructure(path, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)) });
	}

	{
		std::string path(invaderDataFolder + "SpaceInvader_I2_block_pose_1.txt");
		structures.insert({ "I21", new InvaderStructure(path, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)) });
	}
	{
		std::string path(invaderDataFolder + "SpaceInvader_I2_block_pose_2.txt");
		structures.insert({ "I22", new InvaderStructure(path, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)) });
	}

	{
		std::string path(invaderDataFolder + "SpaceInvader_I3_block_pose_1.txt");
		structures.insert({ "I31", new InvaderStructure(path, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)) });
	}
	{
		std::string path(invaderDataFolder + "SpaceInvader_I3_block_pose_2.txt");
		structures.insert({ "I32", new InvaderStructure(path, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)) });
	}

	// UFO
	{
		std::string path(invaderDataFolder + "SpaceInvader_UFO_block.txt");
		structures.insert({ "ufo", new InvaderStructure(path, glm::vec4(0.8f, 0.0f, 0.0f, 1.0f)) });
	}

	// Numbers
	for (int i = 0; i <= 9; i++)
	{
		std::stringstream ss;
		ss << invaderDataFolder << "SpaceInvader_Number_" << i << ".txt";

		std::stringstream ss2;
		ss2 << i;
		structures.insert({ ss2.str(), new InvaderStructure(ss.str(), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)) });
	}
}

void InvaderStructureManager::RenderStructure(Ref<Shader> shader, const std::string& key, const glm::vec3& position)
{
	std::unordered_map<std::string, InvaderStructure*>::iterator it = structures.find(key);
	if (it == structures.end()) return;

	const InvaderStructure* structure = it->second;

	glm::mat4 cubeTransform(1.0f);
	cubeTransform[3] = glm::vec4(position, 1.0f);
	for(int i = 0; i < structure->GetSize(); i++)
	{
		if (structure->structure[i])
		{
			glm::mat4 transform = cubeTransform;
			Renderer::RenderMeshWithColorOverride(shader, invaderCube, transform, structure->color, false, true);
		}
		
		if ((i + 1) % structure->width == 0) // Move down
		{
			
			cubeTransform[3].z = position.z;
			cubeTransform[3].y -= InvaderCubeSize;
		}
		else
		{
			cubeTransform[3].z += InvaderCubeSize;
		}
	}
}

bool InvaderStructureManager::IsCollision(const std::string& structureHandle, const glm::vec3& structurePositon, const glm::vec3& position)
{
	std::unordered_map<std::string, InvaderStructure*>::iterator it = structures.find(structureHandle);
	if (it == structures.end()) return false;

	InvaderStructure* structure = it->second;

	glm::mat4 transform(1.0f);
	transform[3] = glm::vec4(structurePositon, 1.0f);
	for (int i = 0; i < structure->GetSize(); i++)
	{
		if (structure->structure[i]) // There is a cube mesh here
		{
 			if (invaderCube->GetBoundingBox().IsInside(transform, position)) return true; // We have collided with one of the cubes
		}

		if ((i + 1) % structure->width == 0) // Move down
		{

			transform[3].z = structurePositon.z;
			transform[3].y -= InvaderCubeSize;
		}
		else
		{
			transform[3].z += InvaderCubeSize;
		}
	}

	return false;
}

