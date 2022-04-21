#pragma once

#include "SceneMeshData.h"
#include "UUID.h"
#include "EnvironmentMap.h"
#include "Light.h"
#include "Camera.h"
#include "ScenePanel.h"

#include <glm/glm.hpp>

#include <unordered_map>
#include <string>
#include <algorithm>

class Scene
{
public:
	Scene(Ref<Shader> shader);

	void Save(const std::string& path);
	void Load(const std::string& path);

	inline void SetEnvMap(const Ref<EnvironmentMap> envMap) { this->envMap = envMap; }

	void AddLight(const glm::vec3& position);

	void AddMesh(Ref<SceneMeshData> meshData);

	void OnUpdate(Ref<Camera> camera, float deltaTime);

	void NextMesh();
	void PreviousMesh();

	void NextLight();
	void PreviousLight();

	bool showCurrentEdit;
	bool debugMode;
	Ref<Camera> camera;

private:
	std::unordered_map<UUID, Ref<SceneMeshData>> meshes;
	std::vector<Ref<SceneMeshData>> sortedMeshes;
	int transparentEnd;

	int currentMeshIndex;
	int currentLightIndex;

	Ref<EnvironmentMap> envMap;
	std::vector<Ref<Light>> lights;

	Ref<Shader> shader;

	Ref<Mesh> lightMesh;

	static const unsigned int MAX_LIGHTS = 10; // This must match the value in the fragment shader

	ScenePanel scenePanel;
};