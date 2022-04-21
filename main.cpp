#pragma once

#include "GLCommon.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp> 

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector> 
#include <fstream>
#include <sstream>

#include "Camera.h"
#include "Light.h"
#include "EnvironmentMap.h"
#include "AABB.h"
#include "Raycast.h"
#include "Scene.h"
#include "Renderer.h"
#include "MeshManager.h"
#include "TextureManager.h"

#include "InvaderStructureManager.h"
#include "InvaderWorld.h"
#include "PlayerController.h"

const float windowWidth = 1700;
const float windowHeight = 800;
bool editMode = true;

Ref<Camera> camera = CreateRef<Camera>(windowHeight, windowWidth);
float moveSpeed = 10.1f;

static float getRandom(float low, float high)
{
	return low + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (high - low));
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Movement controls
	//if (!editMode)
	//{
	//	if (key == GLFW_KEY_W)
	//	{
	//		camera->position += camera->direction * moveSpeed;
	//	}
	//	if (key == GLFW_KEY_A)
	//	{
	//		// Rotate our camera's direction 90 degrees to the left
	//		glm::vec3 left;
	//		constexpr float theta = glm::radians(90.0f);
	//		left.x = camera->direction.x * cos(theta) + camera->direction.z * sin(theta);
	//		left.y = 0.0f;
	//		left.z = -camera->direction.x * sin(theta) + camera->direction.z * cos(theta);
	//		camera->position += left * moveSpeed;
	//	}
	//	if (key == GLFW_KEY_S)
	//	{
	//		camera->position -= camera->direction * moveSpeed;
	//	}
	//	if (key == GLFW_KEY_D)
	//	{
	//		// Rotate our camera's direction 90 degrees to the right
	//		glm::vec3 right;
	//		constexpr float theta = glm::radians(-90.0f);
	//		right.x = camera->direction.x * cos(theta) + camera->direction.z * sin(theta);
	//		right.y = 0.0f;
	//		right.z = -camera->direction.x * sin(theta) + camera->direction.z * cos(theta);
	//		camera->position += right * moveSpeed;
	//	}
	//	if (key == GLFW_KEY_SPACE)
	//	{
	//		camera->position.y += moveSpeed;
	//	}
	//}

	// Toggle cursor view
	/*if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		editMode = !editMode;
		int cursorOption = editMode ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(window, GLFW_CURSOR, cursorOption);
	}*/
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!editMode)
	{
		camera->MoveCamera(xpos, ypos);
	}
}

int main(void)
{
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
	{
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Initialize our window
	window = glfwCreateWindow(windowWidth, windowHeight, "Midterm", NULL, NULL);

	// Make sure the window initialized properly
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback); // Tell GLFW where our key callbacks are
	glfwSetCursorPosCallback(window, mouse_callback); // Tell GLFW where our mouse callback is

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress); // Give glad this process ID
	glfwSwapInterval(1);

	// Load shader
	std::stringstream ss;
	ss << SOLUTION_DIR << "Extern\\assets\\shaders\\vertexShader.glsl";
	std::string vertexPath = ss.str();
	ss.str("");

	ss << SOLUTION_DIR << "Extern\\assets\\shaders\\fragmentShader.glsl";
	std::string fragmentPath = ss.str();
	ss.str("");

	Ref<Shader> shader = CreateRef<Shader>("Shader#1", vertexPath, fragmentPath);

	float fpsFrameCount = 0.f;
	float fpsTimeElapsed = 0.f;
	float previousTime = static_cast<float>(glfwGetTime());

	Renderer::Initialize(shader);
	Light::InitializeUniforms(shader);
	DiffuseTexture::InitializeUniforms(shader);
	HeightMapTexture::InitializeUniforms(shader);
	DiscardTexture::InitializeUniforms(shader);
	AlphaTexture::InitializeUniforms(shader);

	InvaderStructureManager::Initialize();
	InvaderWorld world;
	PlayerController playerController(world.GetPlayerSpawnPoint());
	world.SetPlayer(&playerController);

	world.Initialize();

	camera->position = glm::vec3(-3026.0f, -80.0f, 800.0f);
	camera->direction = glm::vec3(1.0f, 0.0f, 0.0f);

	const glm::vec3 scorePosition(0.0f, 800.0f, 700.0f);

	// Our actual render loop
	while (!glfwWindowShouldClose(window))
	{
		float currentTime = static_cast<float>(glfwGetTime());
		float deltaTime = currentTime - previousTime;
		previousTime = currentTime;

		// FPS TITLE
		{
			fpsTimeElapsed += deltaTime;
			fpsFrameCount += 1.0f;
			if (fpsTimeElapsed >= 0.03f)
			{
				std::string fps = std::to_string(fpsFrameCount / fpsTimeElapsed);
				std::string ms = std::to_string(1000.f * fpsTimeElapsed / fpsFrameCount);
				std::string newTitle = "FPS: " + fps + "   MS: " + ms;
				glfwSetWindowTitle(window, newTitle.c_str());

	
				fpsTimeElapsed = 0.f;
				fpsFrameCount = 0.f;
			}
		}

		Renderer::BeginFrame(shader, camera);

		// Safety, mostly for first frame
		if (deltaTime == 0.0f)
		{
			deltaTime = 0.03f;
		}

		playerController.OnUpdate(deltaTime);
		world.OnUpdate(deltaTime);

		for (const InvaderObject* enemy : world.GetInvaderObjects())
		{
			InvaderStructureManager::RenderStructure(shader, enemy->GetRenderHandle(), enemy->GetPositionSafe());
		}

		for (const InvaderShield* shield : world.GetShields())
		{
			shield->Render(shader);
		}

		InvaderStructureManager::RenderStructure(shader, playerController.GetRenderHandle(), playerController.GetPosition());

		std::string scoreString = std::to_string(world.GetScore());
		const char* characters = scoreString.c_str();
		glm::vec3 position = scorePosition;
		for (int i = 0; i < scoreString.size(); i++)
		{
			std::string structureHandle(1, characters[i]);
			InvaderStructureManager::RenderStructure(shader, structureHandle, position);
			position.z += InvaderStructureManager::GetInvaderCubeSize() * InvaderStructureManager::GetStructure(structureHandle)->width + InvaderStructureManager::GetInvaderCubeSize();
		}
		//world.GetWorldBounds().Draw(shader, glm::mat4(1.0f)); //For debugging (draws play area)
		
		Renderer::EndFrame();
	}

	glfwDestroyWindow(window); // Clean up the window

	glfwTerminate(); 
	exit(EXIT_SUCCESS);
}