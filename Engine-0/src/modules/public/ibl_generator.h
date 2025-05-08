#pragma once
#include <string>
#include "../../common.h"
#include "framebuffer.h"
#include "shader.h"

struct IBLSettings
{
	uint32_t envSize = 512;
	uint32_t irradianceSize = 32;
	uint32_t prefilterSize = 128;
	uint32_t brdfLUTSize = 512;
	uint32_t maxMipLevels = 5;
	std::string eqrMapPath = "";
};

struct IBLMaps
{
	unsigned int envMap;
	unsigned int irradianceMap;
	unsigned int prefilterMap;
	unsigned int brdfLUT;
};

class IBLGenerator
{
public:
	static IBLMaps Build(const IBLSettings& settings)
	{
		static Shader EQRToCubemap("shaders/IBL/cubemap.vert", "shaders/IBL/eqr_to_cubemap.frag");
		static Shader IrradianceShader("shaders/IBL/cubemap.vert", "shaders/IBL/irradiance_convolution.frag");
		static Shader PrefilterShader("shaders/IBL/cubemap.vert", "shaders/IBL/prefilter_cubemap.frag");
		static Shader IntegratedBRDF("shaders/IBL/brdf.vert", "shaders/IBL/brdf.frag");
		static unsigned int cubeVAO = createCubeVAO();
		static unsigned int frameVAO = createFrameVAO();

		IBLMaps maps{};
		if (settings.eqrMapPath.empty())
		{
			static unsigned int placeholderCubeMap = createPlaceholderCubemap();
			static unsigned int placeholderTexture = TextureLibrary::GetTexture("White Texture - Default").id;
			maps.envMap = placeholderCubeMap;
			maps.irradianceMap = placeholderCubeMap;
			maps.prefilterMap = placeholderCubeMap;
			maps.brdfLUT = placeholderTexture;
			return maps;
		}

		unsigned int eqrTexture = loadHDR(settings.eqrMapPath.c_str(), true);

		// FBO helper
		Framebuffer captureFBO(settings.envSize, settings.envSize);
		captureFBO.attachRenderbuffer(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT24);

		// EQR Environment Cubemap
		glGenTextures(1, &maps.envMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, maps.envMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, settings.envSize, settings.envSize, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // mipmaps to reduce artifacts
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		EQRToCubemap.use();
		EQRToCubemap.setInt("equirectangularMap", 0);
		EQRToCubemap.setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, eqrTexture);

		captureFBO.bind();
		for (unsigned int i = 0; i < 6; i++)
		{
			EQRToCubemap.setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, maps.envMap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}
		captureFBO.unbind();

		// generate mipmaps after the cubemap base texture is set
		glBindTexture(GL_TEXTURE_CUBE_MAP, maps.envMap);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// Irradiance
		glGenTextures(1, &maps.irradianceMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, maps.irradianceMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			// no need for high resolution due to low frequency detailing
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, settings.irradianceSize, settings.irradianceSize, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		captureFBO.editRenderbufferStorage(settings.irradianceSize, settings.irradianceSize, GL_DEPTH_COMPONENT24);

		IrradianceShader.use();
		IrradianceShader.setInt("environmentMap", 0);
		IrradianceShader.setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, maps.envMap);

		captureFBO.bind();
		for (unsigned int i = 0; i < 6; i++)
		{
			IrradianceShader.setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, maps.irradianceMap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}
		captureFBO.unbind();

		// Pre-filtered map
		glGenTextures(1, &maps.prefilterMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, maps.prefilterMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, settings.prefilterSize, settings.prefilterSize, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable trilinear filtering
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		// capture prefilter mip levels
		PrefilterShader.use();
		PrefilterShader.setInt("environmentMap", 0);
		PrefilterShader.setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, maps.envMap);

		captureFBO.bind();
		for (unsigned int mip = 0; mip < settings.maxMipLevels; mip++)
		{
			unsigned int mipWidth = settings.prefilterSize * std::pow(0.5, mip);
			unsigned int mipHeight = settings.prefilterSize * std::pow(0.5, mip);
			glBindRenderbuffer(GL_RENDERBUFFER, captureFBO.getRBO());
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
				mipWidth, mipHeight);
			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(settings.maxMipLevels - 1);
			PrefilterShader.setFloat("roughness", roughness);
			for (unsigned int i = 0; i < 6; i++)
			{
				PrefilterShader.setMat4("view", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, maps.prefilterMap, mip);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				glBindVertexArray(cubeVAO);
				glDrawArrays(GL_TRIANGLES, 0, 36);
				glBindVertexArray(0);
			}
		}
		captureFBO.unbind();

		// Precomputed BRDF
		glGenTextures(1, &maps.brdfLUT);
		glBindTexture(GL_TEXTURE_2D, maps.brdfLUT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, settings.brdfLUTSize, settings.brdfLUTSize, 0, GL_RG, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO.FBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureFBO.getRBO());
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, settings.brdfLUTSize, settings.brdfLUTSize);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, maps.brdfLUT, 0);
		glViewport(0, 0, settings.brdfLUTSize, settings.brdfLUTSize);
		IntegratedBRDF.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindVertexArray(frameVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		captureFBO.unbind();

		glDeleteTextures(1, &eqrTexture);
		return maps;
	}

	static void Destroy(const IBLMaps& maps)
	{
		glDeleteTextures(1, &maps.envMap);
		glDeleteTextures(1, &maps.irradianceMap);
		glDeleteTextures(1, &maps.prefilterMap);
		glDeleteTextures(1, &maps.brdfLUT);
	}
private:
	static unsigned int loadHDR(const char* path, bool flipVertically)
	{
		stbi_set_flip_vertically_on_load(flipVertically);
		int width, height, nrChannels;
		float* data = stbi_loadf(path, &width, &height, &nrChannels, 0);
		if (!data)
		{
			std::cout << "Failed to load HDR image" << std::endl;
			stbi_image_free(data);
			return 0;
		}

		Texture hdrTexture(width, height, GL_RGB32F, GL_RGB, GL_LINEAR, GL_CLAMP_TO_EDGE, data);
		stbi_image_free(data);
		return hdrTexture.id;
	}
};