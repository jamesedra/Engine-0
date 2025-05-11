#pragma once
#include "component_manager.h"
#include "camera.h"
#include "asset_library.h"
#include "renderer.h"
#include "../../common.h"
#include <array>

class RenderSystem
{
private:
	Renderer& renderer;

public:
	RenderSystem(Renderer& renderer) : renderer(renderer) {}

	void RenderGeometry(
		SceneEntityRegistry& sceneRegistry,
		TransformManager& transformManager,
		ShaderManager& shaderManager,
		AssetManager& assetManager,
		MaterialsGroupManager& materialsGroupManager,
		Camera& camera
	)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		renderer.getGBuffer().bind();
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
			shader->setMat4("projection", camera.getProjectionMatrix(WIDTH, HEIGHT, 0.1f, 2500.0f));

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
		renderer.getGBuffer().unbind();
	}

	void RenderShadowPass()
	{

	}

	void RenderSSAO(Camera& camera, unsigned int frameVAO)
	{
		renderer.getSSAOBuffer().bind();
		SSAOData& data = renderer.getSSAOData();
		Shader& ssaoShader = renderer.getSSAOShader();
		SSAOAttachments ssaoTex = renderer.getSSAOAttachments();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		ssaoShader.use();
		int WIDTH = 1600;
		int HEIGHT = 1200;
		ssaoShader.setMat4("projection", camera.getProjectionMatrix(WIDTH, HEIGHT, 0.1f, 2500.0f));
		ssaoShader.setInt("gPositionVS", 0);
		ssaoShader.setInt("gNormalVS", 1);
		ssaoShader.setInt("texNoise", 2);
		// send kernel samples to shader
		for (unsigned int i = 0; i < 64; i++) ssaoShader.setVec3("samples[" + std::to_string(i) + "]", data.kernel[i]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoTex.gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ssaoTex.gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, ssaoTex.ssaoNoiseTex);

		glBindVertexArray(frameVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		renderer.getSSAOBuffer().unbind();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		renderer.getSSAOBlurBuffer().bind();
		Shader& ssaoBlurShader = renderer.getSSAOBlurShader();
		ssaoBlurShader.use();
		ssaoBlurShader.setInt("ssaoInput", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoTex.ssaoColor);
		glBindVertexArray(frameVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		renderer.getSSAOBlurBuffer().unbind();
	}

	void RenderPBR(
		EnvironmentProbeComponent* skyProbe,
		std::vector<EnvironmentProbeComponent*> IBLProbes,
		Camera& camera,
		unsigned int frameVAO
	)
	{
		Shader& pbr = renderer.getPBRShader();

		pbr.use();
		pbr.setVec3("viewPos", camera.getCameraPos());

		// texture passes
		GBufferAttachments gba = renderer.getGAttachments();

		unsigned int unit = 0;
		pbr.setInt("gPosition", unit);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gba.gPosition);

		pbr.setInt("gNormal", ++unit);
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, gba.gNormal);

		pbr.setInt("gAlbedoRoughness", ++unit);
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, gba.gAlbedoRoughness);

		pbr.setInt("gMetallicAO", ++unit);
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, gba.gMetallicAO);

		pbr.setInt("ssaoLUT", ++unit);
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, renderer.getSSAOBlurTexture().id);

		

		const GLuint MAX_PROBES = 4;
		static GLuint placeholderCubemap = createPlaceholderCubemap();
		static GLuint placeholderTexture = TextureLibrary::GetTexture("White Texture - Default").id;

		std::array<GLuint, MAX_PROBES> irradianceMaps { placeholderCubemap };
		std::array<GLuint, MAX_PROBES> prefilterMaps { placeholderCubemap };
		std::array<GLuint, MAX_PROBES> brdfLUTs{ placeholderTexture };

		size_t probeCount = IBLProbes.size();

		int nextSlot = 0;

		// add sky probe first
		if (skyProbe)
		{
			pbr.setVec4("probeData[0]", glm::vec4(skyProbe->position, skyProbe->radius));
			irradianceMaps[0] = skyProbe->maps.irradianceMap;
			prefilterMaps[0] = skyProbe->maps.prefilterMap;
			brdfLUTs[0] = skyProbe->maps.brdfLUT;
			probeCount++;
		}
		else pbr.setVec4("probeData[0]", glm::vec4(FLT_MAX));
		
		nextSlot = 1;

		for (size_t i = 0; i < IBLProbes.size() && i < MAX_PROBES - 1; i++)
		{
			auto* p = IBLProbes[i];

			irradianceMaps[nextSlot] = p->maps.irradianceMap;
			prefilterMaps[nextSlot] = p->maps.prefilterMap;
			brdfLUTs[nextSlot] = p->maps.brdfLUT;

			pbr.setVec4("probeData[" + std::to_string(nextSlot) + "]", glm::vec4(p->position, p->radius));
			nextSlot++;
		}

		pbr.setInt("probeCount", nextSlot);
		
		GLuint firstUnit = ++unit;

		pbr.setSamplerArray("irradianceMap[0]",
			std::vector<GLuint>(irradianceMaps.begin(), irradianceMaps.end()),
			firstUnit,
			GL_TEXTURE_CUBE_MAP);

		firstUnit += MAX_PROBES;

		pbr.setSamplerArray("prefilterMap[0]",
			std::vector<GLuint>(prefilterMaps.begin(), prefilterMaps.end()),
			firstUnit,
			GL_TEXTURE_CUBE_MAP);

		firstUnit += MAX_PROBES;

		pbr.setSamplerArray("brdfLUT[0]",
			std::vector<GLuint>(brdfLUTs.begin(), brdfLUTs.end()),
			firstUnit,
			GL_TEXTURE_2D);

		glBindVertexArray(frameVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void RenderDeferredBrightness(unsigned int frameVAO)
	{
		Framebuffer& brightBuf = renderer.getBrightnessBuffer();
		Shader& brightShader = renderer.getBrightnessShader();
		Texture& tex = renderer.getHDRSceneTex();

		brightBuf.bind();
		brightShader.use();
		brightShader.setInt("hdrScene", 0);
		brightShader.setFloat("threshold", 0.5f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex.id);
		glBindVertexArray(frameVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		brightBuf.unbind();
	}

	void RenderBlur(unsigned int frameVAO)
	{
		BlurBufferAttachments bla = renderer.getBlurParams();

		bool horizontal = true;
		const int blurAmount = 10;
		bla.blurShader.use();
		for (size_t i = 0; i < blurAmount; i++)
		{
			(horizontal ? bla.pongBuf : bla.pingBuf).bind();
			bla.blurShader.setInt("image", 0);
			bla.blurShader.setBool("horizontal", horizontal);
			glActiveTexture(GL_TEXTURE0);
			if (i == 0) glBindTexture(GL_TEXTURE_2D, renderer.getBrightnessTex().id);
			else glBindTexture(GL_TEXTURE_2D, horizontal ? bla.blurHorizontal : bla.blurVertical);
			glBindVertexArray(frameVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			(horizontal ? bla.pongBuf : bla.pingBuf).unbind();
			horizontal = !horizontal;
		}
	}

	void RenderBloom(unsigned int frameVAO)
	{
		renderer.getHDRBuffer().bind();
		Shader& bloomShader = renderer.getBloomShader();
		bloomShader.use();
		bloomShader.setInt("hdrScene", 0);
		bloomShader.setInt("blurBuffer", 1);
		bloomShader.setFloat("exposure", 0.8f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderer.getHDRSceneTex().id);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderer.getBlurHorizontalTex().id);
		glBindVertexArray(frameVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		renderer.getHDRBuffer().unbind();
	}

	void RenderTonemap(unsigned int frameVAO)
	{
		renderer.getTonemapBuffer().bind();
		Shader& tonemap = renderer.getTonemapShader();
		tonemap.use();
		tonemap.setInt("hdrScene", 0);
		tonemap.setFloat("exposure", 0.8f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderer.getHDRSceneTex().id);
		glBindVertexArray(frameVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		renderer.getTonemapBuffer().unbind();
	}

	void RenderComposite(
		EnvironmentProbeComponent* skyProbe,
		Camera& camera, 
		unsigned int frameVAO
	)
	{
		renderer.getCompositeBuffer().bind();
		Shader& compositeShader = renderer.getCompositeShader();
		compositeShader.use();
		int WIDTH = 1600;
		int HEIGHT = 1200;
		glm::mat4 projection = camera.getProjectionMatrix(WIDTH, HEIGHT, 0.1f, 2500.0f);
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 viewNoTrans = glm::mat4(glm::mat3(view));
		glm::mat4 invProjection = glm::inverse(projection);
		glm::mat4 invView = glm::inverse(viewNoTrans);

		compositeShader.setInt("tonemappedScene", 0);
		compositeShader.setInt("sceneDepth", 1);
		compositeShader.setInt("skybox", 2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderer.getTonemapSceneTex().id);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderer.getGDepth().id);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyProbe->maps.envMap);
		compositeShader.setMat4("invProjection", invProjection);
		compositeShader.setMat4("invView", invView);
		glBindVertexArray(frameVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		renderer.getCompositeBuffer().unbind();
	}

	void RenderPostProcess(unsigned int frameVAO)
	{
		renderer.getPPBuffer().bind();
		Shader& ppShader = renderer.getPPShader();
		GBufferAttachments gba = renderer.getGAttachments();
		LBufferAttachments lba = renderer.getLAttachments();

		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		ppShader.use();
		ppShader.setInt("gPosition", 0);
		ppShader.setInt("gNormal", 1);
		ppShader.setInt("gAlbedoRoughness", 2);
		ppShader.setInt("gMetallicAO", 3);
		ppShader.setInt("sceneDepth", 4);
		ppShader.setInt("sceneHDR", 5);
		ppShader.setInt("sceneColor", 6);
		ppShader.setInt("brightPass", 7);
		ppShader.setInt("bloomPass", 8);
		ppShader.setInt("compositePass", 9);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gba.gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gba.gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gba.gAlbedoRoughness);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gba.gMetallicAO);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, renderer.getGDepth().id);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, lba.hdrScene);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, lba.tonemappedScene);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, lba.brightnessPass);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, lba.blurPass);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, lba.compositeScene);
		glBindVertexArray(frameVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		renderer.getPPBuffer().unbind();
	}

	void RenderBufferPass(unsigned int frameVAO)
	{
		renderer.getDebugBuffer().bind();
		GBufferAttachments gba = renderer.getGAttachments();
		Shader& debugBufferShader = renderer.getDebugShader();
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		debugBufferShader.use();
		debugBufferShader.setInt("gPosition", 0);
		debugBufferShader.setInt("gNormal", 1);
		debugBufferShader.setInt("gAlbedoRoughness", 2);
		debugBufferShader.setInt("gMetallicAO", 3);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gba.gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gba.gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gba.gAlbedoRoughness);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gba.gMetallicAO);
		glBindVertexArray(frameVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		renderer.getDebugBuffer().unbind();
	}
};