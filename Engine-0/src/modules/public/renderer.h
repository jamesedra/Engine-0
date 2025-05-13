#pragma once
#include "../../common.h"
#include "framebuffer.h"
#include "texture.h"
#include "utils.h"
#include "loaders.h"

struct GBufferAttachments
{
	unsigned int gPosition;
	unsigned int gNormal;
	unsigned int gAlbedoRoughness;
	unsigned int gMetallicAO;
	unsigned int gPositionVS;
	unsigned int gNormalVS;
};

struct ShadowBufferAttachments
{
	Framebuffer& shadowBuffer;
	Shader& shadowShader;
	unsigned int shadow_width;
	unsigned int shadow_height;
};

struct LBufferAttachments
{
	unsigned int hdrScene;
	unsigned int tonemappedScene;
	unsigned int brightnessPass;
	unsigned int blurPass;
	unsigned int compositeScene;
};

struct BlurBufferAttachments
{
	Framebuffer& pingBuf;
	Framebuffer& pongBuf;
	Shader& blurShader;
	unsigned int blurHorizontal;
	unsigned int blurVertical;
};

struct DebugAttachments
{
	unsigned int position;
	unsigned int normal;
	unsigned int albedo;
	unsigned int metallic;
	unsigned int roughness;
	unsigned int AO;
};

struct SSAOAttachments
{
	unsigned int gPosition;
	unsigned int gNormal;
	unsigned int ssaoNoiseTex;
	unsigned int ssaoColor;
};

class Renderer
{
private:
	// Gbuffer pass
	Framebuffer gBuffer;
	Texture gPosition, gNormal, gAlbedoRoughness, gMetallicAO, gPositionVS, gNormalVS, gDepth;

	// Shadow pass
	Framebuffer shadowBuffer;
	Shader dirShadowDepthShader;
	Texture momentsTex;
	unsigned int shadow_width = 4096, shadow_height = 4096;

	// SSAO pass
	Framebuffer ssaoBuffer, ssaoBlurBuffer;
	Shader ssaoShader, ssaoBlurShader;
	Texture ssaoColor, ssaoBlurColor, ssaoNoiseTexture;
	SSAOData ssaoData;

	// Lighting pass
	Framebuffer hdrBuffer, brightnessBuffer, bloomPingBuffer, bloomPongBuffer, tonemapperBuffer, compositeBuffer, postprocessBuffer;
	Shader pbrBufferShader, brightPassShader, blurShader, bloomShader, tonemapShader, compositeShader, ppShader;
	Texture hdrScene, brightnessPass, blurHorizontal, blurVertical, tonemappedScene, compositeScene, ppScene;

	// Debug pass
	Framebuffer debugBuffer;
	Shader debugShader;
	Texture debugPosition, debugNormal, debugAlbedo, debugMetallic, debugRoughness, debugAO;

public:
	void Initialize(int width, int height)
	{
		// G-Buffer
		gBuffer = Framebuffer(width, height);
		// position color buffer
		gPosition = Texture(width, height, GL_RGBA16F, GL_RGBA);
		gPosition.setTexFilter(GL_NEAREST);
		gBuffer.attachTexture2D(gPosition, GL_COLOR_ATTACHMENT0);
		// normal color buffer
		gNormal = Texture(width, height, GL_RGBA16F, GL_RGBA);
		gNormal.setTexFilter(GL_NEAREST);
		gBuffer.attachTexture2D(gNormal, GL_COLOR_ATTACHMENT1);
		// albedo specular/roughness color buffer
		gAlbedoRoughness = Texture(width, height, GL_RGBA, GL_RGBA);
		gAlbedoRoughness.setTexFilter(GL_NEAREST);
		gBuffer.attachTexture2D(gAlbedoRoughness, GL_COLOR_ATTACHMENT2);
		// metallic and ao buffer
		gMetallicAO = Texture(width, height, GL_RG8, GL_RG);
		gMetallicAO.setTexFilter(GL_NEAREST);
		gBuffer.attachTexture2D(gMetallicAO, GL_COLOR_ATTACHMENT3);
		// view space buffers
		gPositionVS = Texture(width, height, GL_RGBA16F, GL_RGBA);
		gPositionVS.setTexFilter(GL_NEAREST);
		gBuffer.attachTexture2D(gPositionVS, GL_COLOR_ATTACHMENT4);
		gNormalVS = Texture(width, height, GL_RGBA16F, GL_RGBA);
		gNormalVS.setTexFilter(GL_NEAREST);
		gBuffer.attachTexture2D(gNormalVS, GL_COLOR_ATTACHMENT5);
		// z-buffer
		gDepth = Texture(width, height, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT);
		gDepth.setTexFilter(GL_NEAREST);
		gBuffer.attachTexture2D(gDepth, GL_DEPTH_ATTACHMENT);
		// texture and renderbuffer attachments
		gBuffer.bind();

		unsigned int gbuffer_attachments[6] = { 
			GL_COLOR_ATTACHMENT0, 
			GL_COLOR_ATTACHMENT1, 
			GL_COLOR_ATTACHMENT2, 
			GL_COLOR_ATTACHMENT3, 
			GL_COLOR_ATTACHMENT4, 
			GL_COLOR_ATTACHMENT5 
		};
		glDrawBuffers(6, gbuffer_attachments);

		// Shadow framebuffer
		shadowBuffer = Framebuffer(shadow_width, shadow_height);
		momentsTex = Texture(shadow_width, shadow_height, GL_RG32F, GL_RG);
		momentsTex.setTexFilter(GL_NEAREST);
		momentsTex.setTexWrap(GL_CLAMP_TO_EDGE);
		shadowBuffer.attachTexture2D(momentsTex, GL_COLOR_ATTACHMENT0);
		
		shadowBuffer.bind();
		shadowBuffer.attachRenderbuffer(GL_DEPTH_COMPONENT24, GL_DEPTH_ATTACHMENT);
		unsigned int shadow_attachments[1] = { GL_COLOR_ATTACHMENT0 }; // in case of adding more
		glDrawBuffers(1, shadow_attachments);
		 
		// SSAO framebuffer
		ssaoBuffer = Framebuffer(width, height);
		ssaoColor = Texture(width, height, GL_RED, GL_RED);
		ssaoColor.setTexFilter(GL_NEAREST);
		ssaoBuffer.attachTexture2D(ssaoColor, GL_COLOR_ATTACHMENT0);

		// SSAO blur framebuffer
		ssaoBlurBuffer = Framebuffer(width, height);
		ssaoBlurColor = Texture(width, height, GL_RED, GL_RED);
		ssaoBlurColor.setTexFilter(GL_NEAREST);
		ssaoBlurBuffer.attachTexture2D(ssaoBlurColor, GL_COLOR_ATTACHMENT0);

		// SSAO noise texture
		ssaoData = NoiseLoader::CreateSSAONoiseKernel();
		ssaoNoiseTexture = Texture(4, 4, GL_RGBA16F, GL_RGB, GL_NEAREST, GL_REPEAT, &ssaoData.noise[0]);

		// HDR Framebuffer
		hdrBuffer = Framebuffer(width, height);
		hdrScene = Texture(width, height, GL_RGBA16F, GL_RGBA);
		hdrScene.setTexFilter(GL_NEAREST);
		hdrBuffer.attachTexture2D(hdrScene, GL_COLOR_ATTACHMENT0);
		hdrBuffer.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

		// Brightness buffer
		brightnessBuffer = Framebuffer(width, height);
		brightnessPass = Texture(width, height, GL_RGBA16F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
		brightnessBuffer.attachTexture2D(brightnessPass, GL_COLOR_ATTACHMENT0);

		// Bloom buffers
		bloomPingBuffer = Framebuffer(width, height);
		bloomPongBuffer = Framebuffer(width, height);
		blurHorizontal = Texture(width, height, GL_RGBA16F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
		blurVertical = Texture(width, height, GL_RGBA16F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
		bloomPingBuffer.attachTexture2D(blurHorizontal, GL_COLOR_ATTACHMENT0);
		bloomPongBuffer.attachTexture2D(blurVertical, GL_COLOR_ATTACHMENT0);

		// Tonemapper buffer
		tonemapperBuffer = Framebuffer(width, height);
		tonemappedScene = Texture(width, height, GL_RGBA16F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
		tonemapperBuffer.attachTexture2D(tonemappedScene, GL_COLOR_ATTACHMENT0);
		tonemapperBuffer.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

		// Base Composite buffer
		compositeBuffer = Framebuffer(width, height);
		compositeScene = Texture(width, height, GL_RGBA16F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
		compositeBuffer.attachTexture2D(compositeScene, GL_COLOR_ATTACHMENT0);

		// Post process buffer
		postprocessBuffer = Framebuffer(width, height);
		ppScene = Texture(width, height, GL_RGBA16F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
		postprocessBuffer.attachTexture2D(ppScene, GL_COLOR_ATTACHMENT0);
		postprocessBuffer.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

		// Debug buffer
		debugBuffer = Framebuffer(width, height);
		// position
		debugPosition = Texture(width, height, GL_RGBA, GL_RGBA);
		debugPosition.setTexFilter(GL_NEAREST);
		debugBuffer.attachTexture2D(debugPosition, GL_COLOR_ATTACHMENT0);
		// normal
		debugNormal = Texture(width, height, GL_RGBA, GL_RGBA);
		debugNormal.setTexFilter(GL_NEAREST);
		debugBuffer.attachTexture2D(debugNormal, GL_COLOR_ATTACHMENT1);
		// albedo
		debugAlbedo = Texture(width, height, GL_RGBA, GL_RGBA);
		debugAlbedo.setTexFilter(GL_NEAREST);
		debugBuffer.attachTexture2D(debugAlbedo, GL_COLOR_ATTACHMENT2);
		// metallic
		debugMetallic = Texture(width, height, GL_RGBA, GL_RGBA);
		debugMetallic.setTexFilter(GL_NEAREST);
		debugBuffer.attachTexture2D(debugMetallic, GL_COLOR_ATTACHMENT3);
		// roughnesss
		debugRoughness = Texture(width, height, GL_RGBA, GL_RGBA);
		debugRoughness.setTexFilter(GL_NEAREST);
		debugBuffer.attachTexture2D(debugRoughness, GL_COLOR_ATTACHMENT4);
		// ambient occlusion
		debugAO = Texture(width, height, GL_RGBA, GL_RGBA);
		debugAO.setTexFilter(GL_NEAREST);
		debugBuffer.attachTexture2D(debugAO, GL_COLOR_ATTACHMENT5);
		// texture and renderbuffer attachments
		debugBuffer.bind();
		unsigned int debugbuffer_attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
		glDrawBuffers(6, debugbuffer_attachments);
		debugBuffer.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

		// Shaders
		dirShadowDepthShader = Shader("shaders/shadowmapping/dir_depth.vert", "shaders/shadowmapping/dir_depth.frag");
		ssaoShader = Shader("shaders/frame_out.vert", "shaders/ssao/ssao.frag");
		ssaoBlurShader = Shader("shaders/frame_out.vert", "shaders/ssao/ssao_blur.frag");
		pbrBufferShader = Shader("shaders/PBR/pbr_def.vert", "shaders/PBR/pbr_ibl_v2.frag");
		brightPassShader = Shader("shaders/frame_out.vert", "shaders/PBR/bright_pass.frag");
		blurShader = Shader("shaders/frame_out.vert", "shaders/blur/gaussian.frag");
		bloomShader = Shader("shaders/frame_out.vert", "shaders/bloom/bloom.frag");
		tonemapShader = Shader("shaders/frame_out.vert", "shaders/tonemapping/rh_tonemapping.frag");
		compositeShader = Shader("shaders/frame_out.vert", "shaders/composite/composite.frag");
		ppShader = Shader("shaders/frame_out.vert", "shaders/postprocess/pp_celshading.frag");
		debugShader = Shader("shaders/gbuffer/gbuffer_debug_out.vert", "shaders/gbuffer/gbuffer_debug_out.frag");
	}

	void BlitGToLBuffers(int width, int height)
	{
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.FBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrBuffer.FBO);
		glBlitFramebuffer(
			0, 0, width, height,
			0, 0, width, height,
			GL_DEPTH_BUFFER_BIT,
			GL_NEAREST
		);
	}

	GBufferAttachments getGAttachments()
	{
		return { 
			gPosition.id, 
			gNormal.id, 
			gAlbedoRoughness.id, 
			gMetallicAO.id,
			gPositionVS.id,
			gNormalVS.id
		};
	}

	ShadowBufferAttachments getShadowAttachments()
	{
		return {
			shadowBuffer,
			dirShadowDepthShader,
			shadow_width,
			shadow_height
		};
	}

	BlurBufferAttachments getBlurParams()
	{
		return { 
			bloomPingBuffer, 
			bloomPongBuffer, 
			blurShader, 
			blurHorizontal.id, 
			blurVertical.id 
		};
	}

	LBufferAttachments getLAttachments()
	{
		return {
			hdrScene.id,
			tonemappedScene.id,
			brightnessPass.id,
			blurHorizontal.id,
			compositeScene.id
		};
	}

	DebugAttachments getDebugAttachments()
	{
		return {
			debugPosition.id,
			debugNormal.id,
			debugAlbedo.id,
			debugMetallic.id,
			debugRoughness.id,
			debugAO.id
		};
	}

	SSAOAttachments getSSAOAttachments()
	{
		return {
			gPositionVS.id,
			gNormalVS.id,
			ssaoNoiseTexture.id,
			ssaoColor.id
		};
	}

	SSAOData& getSSAOData()
	{
		return ssaoData;
	}

	Texture& getGDepth()
	{
		return gDepth;
	}

	Texture& getShadowMoments()
	{
		return momentsTex;
	}

	Texture& getHDRSceneTex()
	{
		return hdrScene;
	}

	Texture& getBrightnessTex()
	{
		return brightnessPass;
	}

	Texture& getBlurHorizontalTex()
	{
		return blurHorizontal;
	}

	Texture& getTonemapSceneTex()
	{
		return tonemappedScene;
	}

	Texture& getCompositeSceneTex()
	{
		return compositeScene;
	}

	Texture& getPPSceneTex()
	{
		return ppScene;
	}

	Texture& getSSAONoiseTex()
	{
		return ssaoNoiseTexture;
	}

	Texture& getSSAOBlurTexture()
	{
		return ssaoBlurColor;
	}

	Shader& getPBRShader() noexcept
	{
		return pbrBufferShader;
	}

	Shader& getBrightnessShader()
	{
		return brightPassShader;
	}

	Shader& getBloomShader()
	{
		return bloomShader;
	}

	Shader& getTonemapShader()
	{
		return tonemapShader;
	}

	Shader& getCompositeShader()
	{
		return compositeShader;
	}

	Shader& getPPShader()
	{
		return ppShader;
	}

	Shader& getDebugShader()
	{
		return debugShader;
	}

	Shader& getSSAOShader()
	{
		return ssaoShader;
	}

	Shader& getSSAOBlurShader()
	{
		return ssaoBlurShader;
	}
	
	Framebuffer& getGBuffer() noexcept
	{
		return gBuffer;
	};

	Framebuffer& getHDRBuffer() noexcept
	{
		return hdrBuffer;
	};

	Framebuffer& getBrightnessBuffer() noexcept
	{
		return brightnessBuffer;
	}

	Framebuffer& getTonemapBuffer() noexcept
	{
		return tonemapperBuffer;
	}

	Framebuffer& getCompositeBuffer() noexcept
	{
		return compositeBuffer;
	}

	Framebuffer& getPPBuffer() noexcept
	{
		return postprocessBuffer;
	}

	Framebuffer& getDebugBuffer() noexcept
	{
		return debugBuffer;
	}

	Framebuffer& getSSAOBuffer() noexcept
	{
		return ssaoBuffer;
	}

	Framebuffer& getSSAOBlurBuffer() noexcept
	{
		return ssaoBlurBuffer;
	}
};