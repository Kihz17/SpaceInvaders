#pragma once 

#include "pch.h"
#include "Mesh.h"
#include "Shader.h"

#include <glm/glm.hpp>

#include <string>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <iostream>

struct InvaderStructure
{
	InvaderStructure(const std::string& path, const glm::vec4& color)
		: color(color)
	{
		std::ifstream ifs(path);
		if (!ifs.good())
		{
			std::cout << "Could not load InvaderStructure! Path " << path << std::endl;
			return;
		}

		unsigned int longestWidth = 0;
		std::vector<std::string> lines;
		std::string line;
		for (std::string line; std::getline(ifs, line); )
		{
			if (line.size() > longestWidth) longestWidth = line.size();
			lines.push_back(line);
		}

		width = longestWidth;
		height = lines.size();
		structure = new bool[GetSize()];

		unsigned int structureIndex = 0;
		for (int i = 0; i < lines.size(); i++)
		{
			const char* characters = lines[i].c_str();
			for (int j = 0; j < lines[i].size(); j++)
			{
				structure[structureIndex++] = characters[j] == 'X';
			}
		}
	}

	InvaderStructure(const InvaderStructure& copyFrom)
		: width(copyFrom.width),
		height(copyFrom.height),
		color(copyFrom.color)
	{
		this->structure = new bool[copyFrom.GetSize()];
		for (int i = 0; i < copyFrom.GetSize(); i++)
		{
			this->structure[i] = copyFrom.structure[i];
		}
	}

	~InvaderStructure()
	{
		delete[] structure;
	}

	unsigned int GetSize() const { return width * height; }

	unsigned int width, height;
	bool* structure;
	glm::vec4 color;
};

class InvaderStructureManager
{
public:
	static void Initialize();

	static void RenderStructure(Ref<Shader> shader, const std::string& key, const glm::vec3& position);

	inline static float GetInvaderCubeSize() { return InvaderCubeSize; }

	static const InvaderStructure* GetStructure(const std::string& key) { return structures.at(key); }

	static bool IsCollision(const std::string& structureHandle, const glm::vec3& structurePositon, const glm::vec3& position);

	static Ref<Mesh> GetCubeMesh() { return invaderCube; }
private:

	static std::unordered_map<std::string, InvaderStructure*> structures;
	static Ref<Mesh> invaderCube;

	static const float InvaderCubeSize;
};