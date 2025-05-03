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
		std::vector<EnvironmentProbeComponent*> IBLProbes,
		Camera& camera,
		unsigned int frameVAO
	)
	{
		pbrBufferShader.use();
		pbrBufferShader.setVec3("lightPos", lightPos[0], lightPos[1], lightPos[2]);
		pbrBufferShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		pbrBufferShader.setVec3("viewPos", camera.getCameraPos());

		// texture passes
		unsigned int unit = 0;
		pbrBufferShader.setInt("gPosition", unit);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gAttachments.gPosition);

		pbrBufferShader.setInt("gNormal", ++unit);
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, gAttachments.gNormal);

		pbrBufferShader.setInt("gAlbedoRoughness", ++unit);
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, gAttachments.gAlbedoRoughness);

		pbrBufferShader.setInt("gMetallicAO", ++unit);
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, gAttachments.gMetallicAO);

		size_t probeCount = IBLProbes.size();
		pbrBufferShader.setInt("probeCount", probeCount);

		std::vector<GLuint> irradianceMaps;
		std::vector<GLuint> prefilterMaps;
		std::vector<GLuint> brdfLUTs;

		const GLuint MAX_PROBES = 4;
		for (size_t i = 0; i < MAX_PROBES; i++)
		{
			auto* p = IBLProbes[fmin(i, fmax(probeCount-1, 0))];

			irradianceMaps.push_back(p->maps.irradianceMap);
			prefilterMaps.push_back(p->maps.prefilterMap);
			brdfLUTs.push_back(p->maps.brdfLUT);

			pbrBufferShader.setVec3("probePosition[" + std::to_string(i) + "]", p->position);
		}
		
		GLuint firstUnit = ++unit;

		pbrBufferShader.setSamplerArray("irradianceMap[0]",
			irradianceMaps,
			firstUnit,
			GL_TEXTURE_CUBE_MAP);

		firstUnit += MAX_PROBES;

		pbrBufferShader.setSamplerArray("prefilterMap[0]",
			prefilterMaps,
			firstUnit,
			GL_TEXTURE_CUBE_MAP);

		firstUnit += MAX_PROBES;

		pbrBufferShader.setSamplerArray("brdfLUT[0]",
			brdfLUTs,
			firstUnit,
			GL_TEXTURE_2D);

		glBindVertexArray(frameVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
};