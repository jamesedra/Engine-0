#pragma once
#include "component_manager.h"
#include "camera.h"
#include "asset_library.h"

class RenderSystem
{
public:
	void RenderGeometry(
		SceneEntityRegistry& sceneRegistry,
		TransformManager& transformManager,
		ShaderManager& shaderManager,
		AssetManager& assetManager,
		MaterialsGroupManager& materialsGroupManager,
		Camera& camera
	)
	{
		for (Entity entity : sceneRegistry.GetAll())
		{
			AssetComponent* assetComp = assetManager.GetComponent(entity);
			ShaderComponent* shaderComp = shaderManager.GetComponent(entity);
			TransformComponent* transformComp = transformManager.GetComponent(entity);
			MaterialsGroupComponent* materialsGroupComp = materialsGroupManager.GetComponent(entity);

			if (!assetComp || !transformComp || !shaderComp || !materialsGroupComp)
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
			shader->setMat4("view", camera.getViewMatrix());
			int WIDTH = 1600;
			int HEIGHT = 1200;
			shader->setMat4("projection", camera.getProjectionMatrix(WIDTH, HEIGHT, 0.1f, 1000.0f));

			Asset& asset = AssetLibrary::GetAsset(assetComp->assetName);
			auto& parts = asset.parts;

			for (auto& group : materialsGroupComp->materialsGroup)
			{
				group.material.ApplyShaderUniforms(*shader);
				for (size_t index : group.assetPartsIndices)
				{
					parts[index].mesh.Draw(*shader);
				}
			}
		}
	}
};