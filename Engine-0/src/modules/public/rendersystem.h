#pragma once
#include "component_manager.h"
#include "camera.h"
#include "asset_library.h"

class RenderSystem
{
public:
	void Render(
		SceneEntityRegistry& sceneRegistry,
		TransformManager& transformManager,
		ShaderManager& shaderManager,
		AssetManager& assetManager,
		MaterialsManager& materialsManager,
		MaterialsGroupManager& materialsGroupManager,
		Camera& camera
	)
	{
		for (Entity entity : sceneRegistry.GetAll())
		{
			AssetComponent* assetComp = assetManager.GetComponent(entity);
			MaterialsComponent* materialsComp = materialsManager.GetComponent(entity);
			ShaderComponent* shaderComp = shaderManager.GetComponent(entity);
			TransformComponent* transformComp = transformManager.GetComponent(entity);
			MaterialsGroupComponent* materialsGroupComp = materialsGroupManager.GetComponent(entity);

			if (!assetComp || !materialsComp || !transformComp || !shaderComp || !materialsGroupComp)
				continue;

			Shader* shader = shaderComp->shader;
			shader->use();

			glm::mat4 model = glm::mat4(1.0f);

			if (transformComp)
			{
				model = glm::translate(model, transformComp->position);
				model = glm::rotate(model, transformComp->rotation.x, glm::vec3(1, 0, 0));
				model = glm::rotate(model, transformComp->rotation.y, glm::vec3(0, 1, 0));
				model = glm::rotate(model, transformComp->rotation.z, glm::vec3(0, 0, 1));
				model = glm::scale(model, transformComp->scale);
			}

			shader->setMat4("model", model);
			glm::vec3 cameraPos(5.0f, 2.5f, 5.0f);
			glm::vec3 target(0.0f, 0.0f, 0.0f);
			glm::vec3 up(0.0f, 1.0f, 0.0f);
			glm::mat4 view = glm::lookAt(cameraPos, target, up);
			shader->setMat4("view", view);
			shader->setMat4("projection", glm::perspective(glm::radians(45.0f), (float)1600 / (float)1200, 0.1f, 10.0f));

			Asset& asset = AssetLibrary::GetAsset(assetComp->assetName);
			auto& parts = asset.parts;
			// auto& mats = materialsComp->materials;
			for (auto& group : materialsGroupComp->materialsGroup)
			{
				group.material.ApplyShaderUniforms(*shader);
				for (size_t index : group.assetPartsIndices)
				{
					parts[index].mesh.Draw(*shader);
				}
			}

			//// optional sanity check
			//assert(parts.size() == mats.size());

			//for (size_t i = 0; i < parts.size(); ++i)
			//{
			//	mats[i].ApplyShaderUniforms(*shader);
			//	parts[i].mesh.Draw(*shader);
			//}
		}
		
	}
};