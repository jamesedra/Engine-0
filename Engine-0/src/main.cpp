#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "common.h"
#include "modules/public/utils.h"
#include "modules/public/camera.h"
#include "modules/public/framebuffer.h"
#include "modules/public/texture.h"
#include "windows/window.h"
#include "modules/public/render_system.h"
#include "modules/public/factory.h"
#include "modules/public/contexts.h"
#include "modules/public/ibl_generator.h"

constexpr int W_WIDTH = 1600;
constexpr int W_HEIGHT = 1200;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(W_WIDTH, W_HEIGHT, "Engine 0", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Viewport setter
	glViewport(0, 0, W_WIDTH, W_HEIGHT);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glEnable(GL_DEPTH_TEST);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSwapInterval(1); // Enable vsync

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw std::runtime_error("GLAD init failed");

	int maj, min;
	glGetIntegerv(GL_MAJOR_VERSION, &maj);
	glGetIntegerv(GL_MINOR_VERSION, &min);
	std::cout << "OpenGL " << maj << "." << min << " context\n";
	bool HasCompute = maj > 4 || (maj == 4 && min >= 3);

	// Camera settings
	Camera camera(
		glm::vec3(8.0f, 8.0f, 8.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		45.0f
	);
	glfwSetWindowUserPointer(window, &camera);

	// Viewport framebuffer (this is a test framebuffer)
	Framebuffer viewportFrame(W_WIDTH, W_HEIGHT);
	Texture viewportOutTexture(W_WIDTH, W_HEIGHT, GL_RGBA, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
	viewportFrame.attachTexture2D(viewportOutTexture, GL_COLOR_ATTACHMENT0);
	viewportFrame.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

	// Debug G-Buffer (for display)
	Framebuffer debugGBuffer(W_WIDTH, W_HEIGHT);
	// position
	Texture debugPosition(W_WIDTH, W_HEIGHT, GL_RGBA, GL_RGBA);
	debugPosition.setTexFilter(GL_NEAREST);
	debugGBuffer.attachTexture2D(debugPosition, GL_COLOR_ATTACHMENT0);
	// normal
	Texture debugNormal(W_WIDTH, W_HEIGHT, GL_RGBA, GL_RGBA);
	debugNormal.setTexFilter(GL_NEAREST);
	debugGBuffer.attachTexture2D(debugNormal, GL_COLOR_ATTACHMENT1);
	// albedo
	Texture debugAlbedo(W_WIDTH, W_HEIGHT, GL_RGBA, GL_RGBA);
	debugAlbedo.setTexFilter(GL_NEAREST);
	debugGBuffer.attachTexture2D(debugAlbedo, GL_COLOR_ATTACHMENT2);
	// metallic
	Texture debugMetallic(W_WIDTH, W_HEIGHT, GL_RGBA, GL_RGBA);
	debugMetallic.setTexFilter(GL_NEAREST);
	debugGBuffer.attachTexture2D(debugMetallic, GL_COLOR_ATTACHMENT3);
	// roughnesss
	Texture debugRoughness(W_WIDTH, W_HEIGHT, GL_RGBA, GL_RGBA);
	debugRoughness.setTexFilter(GL_NEAREST);
	debugGBuffer.attachTexture2D(debugRoughness, GL_COLOR_ATTACHMENT4);
	// ambient occlusion
	Texture debugAO(W_WIDTH, W_HEIGHT, GL_RGBA, GL_RGBA);
	debugAO.setTexFilter(GL_NEAREST);
	debugGBuffer.attachTexture2D(debugAO, GL_COLOR_ATTACHMENT5);
	// texture and renderbuffer attachments
	debugGBuffer.bind();
	unsigned int debugbuffer_attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
	glDrawBuffers(6, debugbuffer_attachments);
	debugGBuffer.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

	// G-Buffer
	Framebuffer gBuffer(W_WIDTH, W_HEIGHT);
	// position color buffer
	Texture gPosition(W_WIDTH, W_HEIGHT, GL_RGBA16F, GL_RGBA);
	gPosition.setTexFilter(GL_NEAREST);
	gBuffer.attachTexture2D(gPosition, GL_COLOR_ATTACHMENT0);
	// normal color buffer
	Texture gNormal(W_WIDTH, W_HEIGHT, GL_RGBA16F, GL_RGBA);
	gNormal.setTexFilter(GL_NEAREST);
	gBuffer.attachTexture2D(gNormal, GL_COLOR_ATTACHMENT1);
	// albedo specular/roughness color buffer
	Texture gAlbedoRoughness(W_WIDTH, W_HEIGHT, GL_RGBA, GL_RGBA);
	gAlbedoRoughness.setTexFilter(GL_NEAREST);
	gBuffer.attachTexture2D(gAlbedoRoughness, GL_COLOR_ATTACHMENT2);
	// metallic and ao buffer
	Texture gMetallicAO(W_WIDTH, W_HEIGHT, GL_RG8, GL_RG);
	gMetallicAO.setTexFilter(GL_NEAREST);
	gBuffer.attachTexture2D(gMetallicAO, GL_COLOR_ATTACHMENT3);
	// z-buffer
	Texture gDepth(W_WIDTH, W_HEIGHT, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT);
	gDepth.setTexFilter(GL_NEAREST);
	gBuffer.attachTexture2D(gDepth, GL_DEPTH_ATTACHMENT);
	// texture and renderbuffer attachments
	gBuffer.bind();
	unsigned int gbuffer_attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, gbuffer_attachments);
	// since we have a z-buffer, don't use
	// gBuffer.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

	// HDR Frame buffer
	Framebuffer hdrBuffer(W_WIDTH, W_HEIGHT);
	// hdr output
	Texture hdrScene(W_WIDTH, W_HEIGHT, GL_RGBA16F, GL_RGBA);
	hdrScene.setTexFilter(GL_NEAREST);
	hdrBuffer.attachTexture2D(hdrScene, GL_COLOR_ATTACHMENT0);
	hdrBuffer.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

	// Brightness buffer
	Framebuffer	brightnessBuffer(W_WIDTH, W_HEIGHT);
	Texture brightnessPass(W_WIDTH, W_HEIGHT, GL_RGBA16F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
	brightnessBuffer.attachTexture2D(brightnessPass, GL_COLOR_ATTACHMENT0);

	// Bloom buffers
	Framebuffer bloomPingBuffer(W_WIDTH, W_HEIGHT);
	Framebuffer bloomPongBuffer(W_WIDTH, W_HEIGHT);
	Texture blurHorizontal(W_WIDTH, W_HEIGHT, GL_RGBA16F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
	Texture blurVertical(W_WIDTH, W_HEIGHT, GL_RGBA16F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
	bloomPingBuffer.attachTexture2D(blurHorizontal, GL_COLOR_ATTACHMENT0);
	bloomPongBuffer.attachTexture2D(blurVertical, GL_COLOR_ATTACHMENT0);

	// Tonemapper buffer
	Framebuffer tonemapperBuffer(W_WIDTH, W_HEIGHT);
	Texture tonemappedScene(W_WIDTH, W_HEIGHT, GL_RGBA16F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
	tonemapperBuffer.attachTexture2D(tonemappedScene, GL_COLOR_ATTACHMENT0);
	tonemapperBuffer.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

	// Post process buffer
	Framebuffer postprocessBuffer(W_WIDTH, W_HEIGHT);
	Texture ppScene(W_WIDTH, W_HEIGHT, GL_RGBA16F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
	postprocessBuffer.attachTexture2D(ppScene, GL_COLOR_ATTACHMENT0);
	postprocessBuffer.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);
	
	// Textures
	unsigned int tex_diff = loadTexture("resources/textures/brickwall.jpg", true, TextureColorSpace::sRGB);
	unsigned int tex_spec = createDefaultTexture();

	// Object tests
	unsigned int cubeVAO = createCubeVAO();
	unsigned int frameVAO = createFrameVAO();

	// Shaders
	// Deferred Shading
	Shader outShader("shaders/default.vert", "shaders/default.frag");
	Shader outputFrame("shaders/frame_out.vert", "shaders/frame_out.frag");
	Shader debugBufferShader("shaders/gbuffer/gbuffer_debug_out.vert", "shaders/gbuffer/gbuffer_debug_out.frag");
	Shader pbrBufferShader("shaders/PBR/pbr_def.vert", "shaders/PBR/pbr_ibl.frag");
	Shader brightPassShader("shaders/frame_out.vert", "shaders/PBR/bright_pass.frag");
	Shader blurShader("shaders/frame_out.vert", "shaders/blur/gaussian.frag");
	Shader bloomShader("shaders/frame_out.vert", "shaders/bloom/bloom.frag");
	Shader tonemapShader("shaders/frame_out.vert", "shaders/tonemapping/rh_tonemapping.frag");
	Shader ppShader("shaders/frame_out.vert", "shaders/postprocess/pp_celshading.frag");
	// Skybox and IBL Shading
	Shader skyboxShader("shaders/skybox/skybox_default.vert", "shaders/skybox/skybox_default.frag");
	Shader EQRToCubemap("shaders/IBL/cubemap.vert", "shaders/IBL/eqr_to_cubemap.frag");
	Shader IrradianceShader("shaders/IBL/cubemap.vert", "shaders/IBL/irradiance_convolution.frag");
	Shader PrefilterShader("shaders/IBL/cubemap.vert", "shaders/IBL/prefilter_cubemap.frag");
	Shader IntegratedBRDF("shaders/IBL/brdf.vert", "shaders/IBL/brdf.frag");

	// Skybox testing
	stbi_set_flip_vertically_on_load(false);

	std::vector<std::string> faces = {
		"resources/skybox/right.jpg",
		"resources/skybox/left.jpg",
		"resources/skybox/top.jpg",
		"resources/skybox/bottom.jpg",
		"resources/skybox/front.jpg",
		"resources/skybox/back.jpg"
	};

	unsigned int skyboxTexture = loadCubemap(faces);
	stbi_set_flip_vertically_on_load(true);

	// -------------------
	// Component Managers
	EntityManager entityManager;
	IDManager idManager;
	TransformManager transformManager;
	ShaderManager shaderManager;
	AssetManager assetManager;
	MaterialsGroupManager materialsGroupManager;

	// Registries
	SceneEntityRegistry sceneRegistry;

	// Contexts
	WorldContext worldContext(&entityManager, &transformManager, &shaderManager, &assetManager, &materialsGroupManager);
	OutlinerContext outlinerContext(&sceneRegistry, &idManager);
	
	// World Objects
	Entity worldObjectTest = WorldObjectFactory::CreateWorldObject(worldContext, "", "", "resources/objects/backpack/backpack.obj");
	idManager.components[worldObjectTest].ID = "backpack";
	sceneRegistry.Register(worldObjectTest);

	// IBL testing
	IBLSettings IBLsettings{};
	IBLsettings.eqrMapPath = "resources/textures/eqr_maps/newport_loft.hdr";
	IBLMaps IBLmap = IBLGenerator::Build(
		IBLsettings, 
		EQRToCubemap, 
		IrradianceShader, 
		PrefilterShader, 
		IntegratedBRDF, 
		cubeVAO, 
		frameVAO);

	// Systems
	RenderSystem renderSystem;

	// Setup imgui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup imgui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	// Windows
	MainDockWindow mainWindow;
	ViewportWindow viewportWindow;
	OutlinerWindow outlinerWindow(&sceneRegistry, &idManager);
	PropertiesWindow propertiesWindow(&transformManager, &shaderManager, &assetManager, &materialsGroupManager);

	float my_color[4] = { 1.0, 1.0, 1.0, 1.0 };
	static bool viewport_active;
	static bool properties_active;
	static bool outliner_active;
	static ImVec4 color = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
	static int tex_type = 6;
	unsigned int tex_curr;

	static float lightPos[4] = { 0.0f, 0.0f, 0.5f, 0.0f };
	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
		{
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}
		switch (tex_type)
		{
		case 0:
			tex_curr = debugPosition.id;
			break;
		case 1:
			tex_curr = debugNormal.id;
			break;
		case 2:
			tex_curr = debugAlbedo.id;
			break;
		case 3:
			tex_curr = debugMetallic.id;
			break;
		case 4:
			tex_curr = debugRoughness.id;
			break;
		case 5:
			tex_curr = debugAO.id;
			break;
		case 6:
			tex_curr = tonemappedScene.id;
			break;
		default:
			tex_curr = ppScene.id;
			break;
		}

		processInput(window);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// Docking Main Window
		mainWindow.BeginRender();
		mainWindow.EndRender();
		// Viewport window
		viewport_active = viewportWindow.BeginRender();
		if (viewport_active) 
		{ 
			// additional rendering 
			const float window_width = ImGui::GetContentRegionAvail().x;
			const float window_height = ImGui::GetContentRegionAvail().y;
			ImVec2 pos = ImGui::GetCursorScreenPos();

			ImU32 bg_color = IM_COL32(0.0*255, 0.0*255, 0.0*255, 1.0*255);
			ImGui::GetWindowDrawList()->AddRectFilled(
				pos,
				ImVec2(pos.x + window_width, pos.y + window_height),
				bg_color
			);
			const float fb_aspect = W_WIDTH / (float)W_HEIGHT;
			float win_aspect = window_width / window_height;

			float display_width, display_height;
			if (win_aspect > fb_aspect)
			{
				display_height = window_height;
				display_width = window_height * fb_aspect;
			}
			else
			{
				display_width = window_width;
				display_height = window_width / fb_aspect;
			}

			float offset_x = (window_width - display_width) * 0.5f;
			float offset_y = (window_height - display_height) * 0.5f;

			ImGui::GetWindowDrawList()->AddImage(
				tex_curr,           
				ImVec2(pos.x + offset_x, pos.y + offset_y),
				ImVec2(pos.x + offset_x + display_width, pos.y + offset_y + display_height),
				ImVec2(0, 1),
				ImVec2(1, 0)
			);
			
			viewportWindow.EndRender();
		}
		outliner_active = outlinerWindow.BeginRender();
		if (outliner_active)
		{
			// additional rendering
			outlinerWindow.EndRender();
		}
		properties_active = propertiesWindow.BeginRender();
		if (properties_active)
		{
			// additional rendering
			propertiesWindow.SetExpandedEntity(outlinerWindow.GetSelectedEntity());

			// tentative placement
			ImGui::RadioButton("World Position", &tex_type, 0);
			ImGui::RadioButton("World Normal", &tex_type, 1);
			ImGui::RadioButton("Base Color", &tex_type, 2);
			ImGui::RadioButton("Metallic", &tex_type, 3);
			ImGui::RadioButton("Roughness", &tex_type, 4);
			ImGui::RadioButton("Ambient Occlusion", &tex_type, 5);
			ImGui::RadioButton("Lit", &tex_type, 6);
			ImGui::RadioButton("Cel Shaded", &tex_type, 7);

			ImGui::DragFloat3("Light Position", lightPos, 0.5f, -50.0f, 50.0f);
			propertiesWindow.EndRender();
		}

		// GBuffer pass
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		gBuffer.bind();
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderSystem.RenderGeometry(sceneRegistry, transformManager, shaderManager, assetManager, materialsGroupManager, camera);
		gBuffer.unbind();

		// deferred shading stage
		glDisable(GL_DEPTH_TEST);

		if (tex_type > 5)
		{
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.FBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrBuffer.FBO);
			glBlitFramebuffer(
				0, 0, W_WIDTH, W_HEIGHT,
				0, 0, W_WIDTH, W_HEIGHT,
				GL_DEPTH_BUFFER_BIT,
				GL_NEAREST
			);

			// PBR shading
			hdrBuffer.bind();
			pbrBufferShader.use();
			pbrBufferShader.setVec3("lightPos", lightPos[0], lightPos[1], lightPos[2]);
			pbrBufferShader.setVec3("lightColor", lightColor);
			pbrBufferShader.setVec3("viewPos", glm::vec3(5.0f, 2.5f, 5.0f)); // tentative
			pbrBufferShader.setInt("gPosition", 0);
			pbrBufferShader.setInt("gNormal", 1);
			pbrBufferShader.setInt("gAlbedoRoughness", 2);
			pbrBufferShader.setInt("gMetallicAO", 3);
			pbrBufferShader.setInt("irradianceMap", 4);
			pbrBufferShader.setInt("prefilterMap", 5);
			pbrBufferShader.setInt("brdfLUT", 6);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition.id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal.id);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, gAlbedoRoughness.id);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, gMetallicAO.id);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_CUBE_MAP, IBLmap.irradianceMap);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_CUBE_MAP, IBLmap.prefilterMap);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, IBLmap.brdfLUT);
			glBindVertexArray(frameVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			hdrBuffer.unbind();

			// Brightness pass
			brightnessBuffer.bind();
			brightPassShader.use();
			brightPassShader.setInt("hdrScene", 0);
			brightPassShader.setFloat("threshold", 0.5f);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, hdrScene.id);
			glBindVertexArray(frameVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			brightnessBuffer.unbind();

			// Blur shading
			bool horizontal = true;
			const int blurAmount = 10;
			blurShader.use();
			for (size_t i = 0; i < blurAmount; i++)
			{
				(horizontal ? bloomPongBuffer : bloomPingBuffer).bind();
				blurShader.setInt("image", 0);
				blurShader.setBool("horizontal", horizontal);
				glActiveTexture(GL_TEXTURE0);
				if (i == 0) glBindTexture(GL_TEXTURE_2D, brightnessPass.id);
				else glBindTexture(GL_TEXTURE_2D, horizontal ? blurHorizontal.id : blurVertical.id);
				glBindVertexArray(frameVAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				(horizontal ? bloomPongBuffer : bloomPingBuffer).unbind();
				horizontal = !horizontal;
			}

			// Bloom shading
			hdrBuffer.bind();
			bloomShader.use();
			bloomShader.setInt("hdrScene", 0);
			bloomShader.setInt("blurBuffer", 1);
			bloomShader.setFloat("exposure", 0.8f);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, hdrScene.id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, blurHorizontal.id);
			glBindVertexArray(frameVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			hdrBuffer.unbind();

			// Tone mapping
			tonemapperBuffer.bind();
			tonemapShader.use();
			tonemapShader.setInt("hdrScene", 0);
			tonemapShader.setFloat("exposure", 0.8f);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, hdrScene.id);
			glBindVertexArray(frameVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			tonemapperBuffer.unbind();

			// Post processing
			postprocessBuffer.bind();
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
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition.id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal.id);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, gAlbedoRoughness.id);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, gMetallicAO.id);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, gDepth.id);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, hdrScene.id);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, tonemappedScene.id);
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, brightnessPass.id);
			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, blurHorizontal.id);
			glBindVertexArray(frameVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			postprocessBuffer.unbind();

			glEnable(GL_DEPTH_TEST);

			// skybox
			//glFrontFace(GL_CW);
			//glDepthFunc(GL_LEQUAL);
			//glDepthMask(GL_FALSE);
			//skyboxShader.use();
			//glm::vec3 cameraPos(5.0f, 2.5f, 5.0f);
			//glm::vec3 target(0.0f, 0.0f, 0.0f);
			//glm::vec3 up(0.0f, 1.0f, 0.0f);
			//glm::mat4 view = glm::lookAt(cameraPos, target, up);
			//skyboxShader.setMat4("projection", glm::perspective(glm::radians(45.0f), (float)1600 / (float)1200, 0.1f, 10.0f));
			//skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
			//skyboxShader.setInt("skybox", 0);
			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
			//glBindVertexArray(cubeVAO);
			//glDrawArrays(GL_TRIANGLES, 0, 36);
			//glDepthFunc(GL_LESS);
			//glFrontFace(GL_CCW);
			//glDepthMask(GL_TRUE);
		}
		else if (tex_type <= 5)
		{
			// Debug GBuffer pass
			debugGBuffer.bind();
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			debugBufferShader.use();
			debugBufferShader.setInt("gPosition", 0);
			debugBufferShader.setInt("gNormal", 1);
			debugBufferShader.setInt("gAlbedoRoughness", 2);
			debugBufferShader.setInt("gMetallicAO", 3);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition.id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal.id);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, gAlbedoRoughness.id);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, gMetallicAO.id);
			glBindVertexArray(frameVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			debugGBuffer.unbind();
		}

		glEnable(GL_DEPTH_TEST);
		
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
