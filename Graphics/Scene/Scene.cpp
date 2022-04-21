#include "Scene.h"
#include "MeshManager.h"
#include "Renderer.h"
#include "YAMLOverloads.h"

#include <sstream>
#include <fstream>

Scene::Scene(Ref<Shader> shader)
	: shader(shader), 
	transparentEnd(-1), 
	debugMode(false), 
	scenePanel(this), 
	currentMeshIndex(0), 
	currentLightIndex(0), 
	showCurrentEdit(true),
	camera(NULL)
{
	std::stringstream ss;
	ss << SOLUTION_DIR << "Extern\\assets\\models\\ISO_Sphere.ply";
	this->lightMesh = MeshManager::LoadMesh(ss.str());
}

void Scene::Save(const std::string& path)
{
	if (path.empty())
	{
		return;
	}

	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Scene" << YAML::Value << "Untitled";

	if (this->camera)
	{
		out << YAML::Key << "Camera" << YAML::Value;
		out << YAML::BeginMap;
		out << YAML::Key << "Position" << YAML::Value << this->camera->position;
		out << YAML::Key << "Direction" << YAML::Value << this->camera->direction;
		out << YAML::Key << "Yaw" << YAML::Value << this->camera->yaw;
		out << YAML::Key << "Pitch" << YAML::Value << this->camera->pitch;
		out << YAML::EndMap;
	}

	// Save EnvMap
	if (this->envMap)
	{
		out << YAML::Key << "EnvironmentMap" << YAML::Value;
		this->envMap->Save(out);
	}

	// Save lights
	out << YAML::Key << "Lights" << YAML::Value << YAML::BeginSeq;
	for (const Ref<Light>& light : this->lights)
	{
		light->Save(out);
	}
	out << YAML::EndSeq;

	// Save meshes
	out << YAML::Key << "Meshes" << YAML::Value << YAML::BeginSeq;
	std::unordered_map<UUID, Ref<SceneMeshData>>::iterator meshIt;
	for (meshIt = this->meshes.begin(); meshIt != this->meshes.end(); meshIt++)
	{
		meshIt->second->Save(out);
	}
	out << YAML::EndSeq;

	out << YAML::EndMap;

	std::ofstream ofs(path);
	ofs << out.c_str();
}

void Scene::Load(const std::string& path)
{
	std::ifstream ifs(path);
	if (!ifs.good())
	{
		std::cout << "Scene does not exist!" << std::endl;
		return;
	}

	this->meshes.clear();
	this->sortedMeshes.clear();
	this->lights.clear();
	this->transparentEnd = -1;
	this->currentMeshIndex = 0;
	this->scenePanel.SetMeshData(NULL);

	std::stringstream ss;
	ss << ifs.rdbuf();

	shader->Bind();
	
	YAML::Node root = YAML::Load(ss.str());
	if (!root["Scene"])
	{
		std::cout << "Error loading scene file " << path << std::endl;
		return;
	}

	std::string sceneName = root["Scene"].as<std::string>();

	const YAML::Node& cameraNode = root["Camera"];
	if (cameraNode && this->camera)
	{
		this->camera->position = cameraNode["Position"].as<glm::vec3>();
		this->camera->direction = cameraNode["Direction"].as<glm::vec3>();
		this->camera->yaw = cameraNode["Yaw"].as<float>();
		this->camera->pitch = cameraNode["Pitch"].as<float>();
	}

	// Load Env map
	const YAML::Node& envMapNode = root["EnvironmentMap"];
	if (envMapNode)
	{		
		this->envMap = EnvironmentMap::StaticLoad(envMapNode);
	}

	// Load Lights
	const YAML::Node& lightNode = root["Lights"];
	if (lightNode)
	{
		YAML::const_iterator it;
		for (it = lightNode.begin(); it != lightNode.end(); it++)
		{
			YAML::Node childNode = (*it);
			Ref<Light> light = Light::StaticLoad(childNode);

			light->SendToShader();
			this->lights.push_back(light);
		}
	}

	const YAML::Node& meshNode = root["Meshes"];
	if (meshNode)
	{
		YAML::const_iterator it;
		for (it = meshNode.begin(); it != meshNode.end(); it++)
		{
			const YAML::Node& node = (*it);
			Ref<SceneMeshData> meshData = SceneMeshData::StaticLoad(node);
			AddMesh(meshData);
		}
	}
}

void Scene::AddLight(const glm::vec3& position)
{
	int lightIndex = this->lights.size();
	if (lightIndex >= MAX_LIGHTS)
	{
		std::cout << "The maximum number of lights has been reached. You must increase the MAX_LIGHTS value in LightManager.h AND the fragment shader to inccrease the number of lights." << std::endl;
		return;
	}

	Ref<Light> light = CreateRef<Light>(lightIndex);
	light->position = glm::vec4(position, 1.0f);

	shader->Bind();
	light->SendToShader();
	this->lights.push_back(light);
}

void Scene::AddMesh(Ref<SceneMeshData> meshData)
{
	this->meshes.insert(std::make_pair(meshData->uuid, meshData) );

	if (!this->sortedMeshes.empty() && meshData->alphaTransparency < 1.0f) // We have to add transparent objects to the front so we can handle blend transparency
	{
		this->sortedMeshes.insert(this->sortedMeshes.begin(), meshData);
		transparentEnd++;
	}
	else
	{
		this->sortedMeshes.push_back(meshData);
	}
}

void Scene::OnUpdate(Ref<Camera> camera, float deltaTime)
{
	glDisable(GL_DEPTH);
	glDisable(GL_DEPTH_TEST);

	// Draw environment map
	if (envMap)
	{
		this->envMap->Draw(this->shader, camera->position, glm::vec3(10.0f, 10.0f, 10.0f));
	}

	// Draw meshes
	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);

	for (int i = 1; i <= this->transparentEnd; i++) // Sort the transparent meshes (use insertion sort here since they are sorted the majority of the time)
	{
		int j = i - 1;

		Ref<SceneMeshData> cachedMeshData = this->sortedMeshes[i];
		glm::vec3 difference = camera->position - cachedMeshData->position;
		float currentDistance = glm::dot(difference, difference);

		glm::vec3 difference2 = camera->position - this->sortedMeshes[j]->position;
		float toCompare = glm::dot(difference2, difference2);

		while (j >= 0 && toCompare < currentDistance)
		{
			this->sortedMeshes[j + 1] = this->sortedMeshes[j];
			j = j - 1;
		}

		this->sortedMeshes[j + 1] = cachedMeshData;
	}

	glDisable(GL_BLEND);

	// Render opaque meshes
	int startIndex = this->transparentEnd == -1 ? 0 : this->transparentEnd + 1;
	std::vector<int> changedAlphaValues;
	for (int i = startIndex; i < this->sortedMeshes.size(); i++)
	{
		Ref<SceneMeshData> meshData = this->sortedMeshes[i];
		glm::mat4 transform(1.0f);
		transform *= glm::translate(glm::mat4(1.0f), meshData->position);
		transform *= glm::rotate(glm::mat4(1.0f), meshData->orientation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		transform *= glm::rotate(glm::mat4(1.0f), meshData->orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		transform *= glm::rotate(glm::mat4(1.0f), meshData->orientation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		transform *= glm::scale(glm::mat4(1.0f), meshData->scale);

		if (meshData->alphaTransparency < 1.0f || meshData->hasAlphaTransparentTexture) // Alpha transparency is now below 1, we need to flag this object to be sorted
		{
			changedAlphaValues.push_back(i);
		}

		if (meshData == this->scenePanel.GetEditMesh() && this->showCurrentEdit)
		{
			Renderer::RenderMeshWithColorOverride(shader, meshData->mesh, transform, glm::vec3(0.0f, 1.0f, 0.0f), this->debugMode, true);
		}
		else
		{
			Renderer::RenderMeshWithTextures(shader, meshData->mesh, meshData->textures, transform, meshData->alphaTransparency, this->debugMode);
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i <= this->transparentEnd; i++)
	{
		Ref<SceneMeshData> meshData = this->sortedMeshes[i];
		glm::mat4 transform(1.0f);
		transform *= glm::translate(glm::mat4(1.0f), meshData->position);
		transform *= glm::rotate(glm::mat4(1.0f), meshData->orientation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		transform *= glm::rotate(glm::mat4(1.0f), meshData->orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		transform *= glm::rotate(glm::mat4(1.0f), meshData->orientation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		transform *= glm::scale(glm::mat4(1.0f), meshData->scale);

		if (meshData == this->scenePanel.GetEditMesh() && this->showCurrentEdit)
		{
			Renderer::RenderMeshWithColorOverride(shader, meshData->mesh, transform, glm::vec3(0.0f, 1.0f, 0.0f), this->debugMode, true);
		}
		else
		{
			Renderer::RenderMeshWithTextures(shader, meshData->mesh, meshData->textures, transform, meshData->alphaTransparency, this->debugMode);
		}
	}

	glDisable(GL_BLEND);

	if (!changedAlphaValues.empty())
	{
		for (int& i : changedAlphaValues) // Ad changed alpha meshes to the front of the vector
		{
			std::rotate(this->sortedMeshes.begin(), this->sortedMeshes.begin() + i, this->sortedMeshes.begin() + (i + 1));
			//std::rotate(this->sortedMeshes.rend() - i, this->sortedMeshes.rend() - i, this->sortedMeshes.rend() - 0);
		}
		this->transparentEnd += changedAlphaValues.size(); // Make sure we update our transparent end index
	}

	// Draw lights
	if (debugMode)
	{
		for (const Ref<Light>& light : this->lights)
		{
			glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(light->position));
			{
				float distTo95Percent = Light::CalcApproxDistFromAtten(0.95f, 0.01, 10000.0f, light->attenuation.x, light->attenuation.y, light->attenuation.z, 50);
				glm::mat4 transform(1.0f);
				transform *= translate;
				transform *= glm::scale(glm::mat4(1.0f), glm::vec3(distTo95Percent, distTo95Percent, distTo95Percent));
				Renderer::RenderMeshWithColorOverride(this->shader, this->lightMesh, transform, glm::vec3(1.0f, 0.0f, 0.0f), this->debugMode, true);
			}
			{
				float distTo50Percent = Light::CalcApproxDistFromAtten(0.5f, 0.01, 10000.0f, light->attenuation.x, light->attenuation.y, light->attenuation.z, 50);
				glm::mat4 transform(1.0f);
				transform *= translate;
				transform *= glm::scale(glm::mat4(1.0f), glm::vec3(distTo50Percent, distTo50Percent, distTo50Percent));
				Renderer::RenderMeshWithColorOverride(this->shader, this->lightMesh, transform, glm::vec3(1.0f, 1.0f, 0.0f), this->debugMode, true);
			}
			{
				float distTo25Percent = Light::CalcApproxDistFromAtten(0.25f, 0.01, 10000.0f, light->attenuation.x, light->attenuation.y, light->attenuation.z, 50);
				glm::mat4 transform(1.0f);
				transform *= translate;
				transform *= glm::scale(glm::mat4(1.0f), glm::vec3(distTo25Percent, distTo25Percent, distTo25Percent));
				Renderer::RenderMeshWithColorOverride(this->shader, this->lightMesh, transform, glm::vec3(0.0f, 1.0f, 0.0f), this->debugMode, true);
			}
			{
				float distTo5Percent = Light::CalcApproxDistFromAtten(0.25f, 0.01, 10000.0f, light->attenuation.x, light->attenuation.y, light->attenuation.z, 50);
				glm::mat4 transform(1.0f);
				transform *= translate;
				transform *= glm::scale(glm::mat4(1.0f), glm::vec3(distTo5Percent, distTo5Percent, distTo5Percent));
				Renderer::RenderMeshWithColorOverride(this->shader, this->lightMesh, transform, glm::vec3(0.0f, 0.0f, 1.0f), this->debugMode, true);
			}

			// TODO: Draw attenuation and other light attributes to screen
			glm::mat4 transform(1.0f);
			transform *= translate;
			Renderer::RenderMeshWithColorOverride(this->shader, this->lightMesh, transform, glm::vec3(1.0f, 1.0f, 1.0f), this->debugMode, true);
		}
	}

	scenePanel.OnUpdate(deltaTime);
}

void Scene::NextMesh()
{
	if (this->sortedMeshes.empty())
	{
		this->scenePanel.SetMeshData(NULL);
		return;
	}

	this->currentMeshIndex = std::min(this->currentMeshIndex + 1, (int)(this->sortedMeshes.size() - 1));
	this->scenePanel.SetMeshData(sortedMeshes[currentMeshIndex]);
}

void Scene::PreviousMesh()
{
	if (this->sortedMeshes.empty())
	{
		this->scenePanel.SetMeshData(NULL);
		return;
	}

	this->currentMeshIndex = std::max(this->currentMeshIndex - 1, 0);
	this->scenePanel.SetMeshData(sortedMeshes[currentMeshIndex]);
}

void Scene::NextLight()
{
	if (this->lights.empty())
	{
		this->scenePanel.SetLight(NULL);
		return;
	}

	this->currentLightIndex = std::min(this->currentLightIndex + 1, (int)(this->lights.size() - 1));
	this->scenePanel.SetLight(lights[currentLightIndex]);
}

void Scene::PreviousLight()
{
	if (this->lights.empty())
	{
		this->scenePanel.SetLight(NULL);
		return;
	}

	this->currentLightIndex = std::max(this->currentLightIndex - 1, 0);
	this->scenePanel.SetLight(lights[currentLightIndex]);
}