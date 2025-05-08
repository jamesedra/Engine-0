#pragma once
#include "../../common.h"
#include "framebuffer.h"
#include "texture.h"
#include "utils.h"

struct GBufferAttachments
{
	unsigned int gPosition;
	unsigned int gNormal;
	unsigned int gAlbedoRoughness;
	unsigned int gMetallicAO;
};

class Renderer
{
private:
	// Gbuffer pass
	Framebuffer gBuffer;
	Texture gPosition, gNormal, gAlbedoRoughness, gMetallicAO, gDepth;
	// Lighting pass
	Framebuffer hdrBuffer, brightnessBuffer, bloomPingBuffer, bloomPongBuffer, tonemapperBuffer, compositeBuffer, postprocessBuffer;
	Texture hdrScene, brightnessPass, blurHorizontal, blurVertical, tonemappedScene, compositeScene, ppScene;
	// Lighting shaders
	Shader pbrBufferShader, brightPassShader, blurShader, bloomShader, tonemapShader, compositeShader, ppShader;

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
		// z-buffer
		gDepth = Texture(width, height, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT);
		gDepth.setTexFilter(GL_NEAREST);
		gBuffer.attachTexture2D(gDepth, GL_DEPTH_ATTACHMENT);
		// texture and renderbuffer attachments
		gBuffer.bind();
		unsigned int gbuffer_attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, gbuffer_attachments);

		// HDR Frame buffer
		hdrBuffer = Framebuffer(width, height);
		// hdr output
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

		// Shaders
		pbrBufferShader = Shader("shaders/PBR/pbr_def.vert", "shaders/PBR/pbr_ibl_v2.frag");
		brightPassShader = Shader("shaders/frame_out.vert", "shaders/PBR/bright_pass.frag");
		blurShader = Shader("shaders/frame_out.vert", "shaders/blur/gaussian.frag");
		bloomShader = Shader("shaders/frame_out.vert", "shaders/bloom/bloom.frag");
		tonemapShader = Shader("shaders/frame_out.vert", "shaders/tonemapping/rh_tonemapping.frag");
		compositeShader = Shader("shaders/frame_out.vert", "shaders/composite/composite.frag");
		ppShader = Shader("shaders/frame_out.vert", "shaders/postprocess/pp_celshading.frag");
	}
};