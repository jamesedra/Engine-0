#pragma once
#include "component_manager.h"
#include "camera.h"
#include "asset_library.h"

struct GBufferAttachments
{
	unsigned int gPosition;
	unsigned int gNormal;
	unsigned int gAlbedoRoughness;
	unsigned int gMetallicAO;
};

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

	void RenderDeferredPBR(
		Shader& pbrBufferShader,
		float lightPos[4], // tentative
		GBufferAttachments& gAttachments,
		std::vector<IBLMaps>& IBLmaps,
		Camera& camera,
		unsigned int frameVAO
	)
	{
		pbrBufferShader.use();
		pbrBufferShader.setVec3("lightPos", lightPos[0], lightPos[1], lightPos[2]);
		pbrBufferShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		pbrBufferShader.setVec3("viewPos", camera.getCameraPos());
		pbrBufferShader.setInt("gPosition", 0);
		pbrBufferShader.setInt("gNormal", 1);
		pbrBufferShader.setInt("gAlbedoRoughness", 2);
		pbrBufferShader.setInt("gMetallicAO", 3);
		pbrBufferShader.setInt("irradianceMap", 4);
		pbrBufferShader.setInt("prefilterMap", 5);
		pbrBufferShader.setInt("brdfLUT", 6);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gAttachments.gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gAttachments.gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAttachments.gAlbedoRoughness);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gAttachments.gMetallicAO);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, IBLmaps[0].irradianceMap);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, IBLmaps[0].prefilterMap);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, IBLmaps[0].brdfLUT);
		glBindVertexArray(frameVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
};