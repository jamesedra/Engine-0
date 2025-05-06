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
#include "modules/public/probe_system.h"
#include "modules/public/factory.h"
#include "modules/public/contexts.h"
#include "modules/public/ibl_generator.h"

constexpr int W_WIDTH = 1600;
constexpr int W_HEIGHT = 1200;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// mouse stuff
bool firstMouse = false;
float lastX = 400;
float lastY = 300;
float pitch = 0.0f;
float yaw = -90.0f;
float zoom = 45.0f;

// adjusts the viewport when user resizes it
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// process input in the renderer
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

static bool gViewportCaptured = false;

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

	// Base Composite buffer
	Framebuffer compositeBuffer(W_WIDTH, W_HEIGHT);
	Texture compositeScene(W_WIDTH, W_HEIGHT, GL_RGBA16F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
	compositeBuffer.attachTexture2D(compositeScene, GL_COLOR_ATTACHMENT0);

	// Post process buffer
	Framebuffer postprocessBuffer(W_WIDTH, W_HEIGHT);
	Texture ppScene(W_WIDTH, W_HEIGHT, GL_RGBA16F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
	postprocessBuffer.attachTexture2D(ppScene, GL_COLOR_ATTACHMENT0);
	postprocessBuffer.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

	GBufferAttachments gAttachments
	{
		gPosition.id,
		gNormal.id,
		gAlbedoRoughness.id,
		gMetallicAO.id 
	};
	
	// Textures
	unsigned int tex_diff = loadTexture("resources/textures/brickwall.jpg", true, TextureColorSpace::sRGB);
	unsigned int tex_spec = createDefaultTexture();

	// Object tests
	unsigned int cubeVAO = createCubeVAO();
	unsigned int frameVAO = createFrameVAO();

	// Shaders
	// Deferred Shading
	Shader outShader("shaders/frame_out.vert", "shaders/frame_out.frag");
	Shader outputFrame("shaders/frame_out.vert", "shaders/frame_out.frag");
	Shader debugBufferShader("shaders/gbuffer/gbuffer_debug_out.vert", "shaders/gbuffer/gbuffer_debug_out.frag");
	Shader pbrBufferShader("shaders/PBR/pbr_def.vert", "shaders/PBR/pbr_ibl_v1.frag");
	Shader brightPassShader("shaders/frame_out.vert", "shaders/PBR/bright_pass.frag");
	Shader blurShader("shaders/frame_out.vert", "shaders/blur/gaussian.frag");
	Shader bloomShader("shaders/frame_out.vert", "shaders/bloom/bloom.frag");
	Shader tonemapShader("shaders/frame_out.vert", "shaders/tonemapping/rh_tonemapping.frag");
	Shader compositeShader("shaders/frame_out.vert", "shaders/composite/composite.frag");
	Shader ppShader("shaders/frame_out.vert", "shaders/postprocess/pp_celshading.frag");
	// Skybox and IBL Shading
	Shader skyboxShader("shaders/skybox/skybox_default.vert", "shaders/skybox/skybox_default.frag");

	// -------------------
	// Component Managers
	EntityManager entityManager;
	IDManager idManager;
	TransformManager transformManager;
	ShaderManager shaderManager;
	AssetManager assetManager;
	MaterialsGroupManager materialsGroupManager;
	EnvironmentProbeManager probeManager;

	// Registries
	SceneEntityRegistry sceneRegistry;

	// Contexts
	WorldContext worldContext(&entityManager, &transformManager, &shaderManager, &assetManager, &materialsGroupManager);
	OutlinerContext outlinerContext(&sceneRegistry, &idManager);
	
	// World Objects
	Entity backpackEntity = WorldObjectFactory::CreateWorldObject(worldContext, "", "", "resources/objects/backpack/backpack.obj");
	idManager.components[backpackEntity].ID = "backpack";
	sceneRegistry.Register(backpackEntity);
	Entity floorEntity = WorldObjectFactory::CreateWorldObject(worldContext, "", "", "");
	idManager.components[floorEntity].ID = "floor";
	transformManager.components[floorEntity].position = glm::vec3(0.0f, -2.0f, 0.0f);
	transformManager.components[floorEntity].scale = glm::vec3(20.0f, 0.5f, 20.0f);
	sceneRegistry.Register(floorEntity);

	// IBL testing
	// probe entity test
	IBLSettings skyboxIBLSettings = ProbeLibrary::GetSettings("resources/textures/eqr_maps/kloofendal_43d_clear_puresky_2k.hdr");
	Entity skyboxEntity = WorldObjectFactory::CreateEnvironmentProbe(entityManager, probeManager, idManager, "skybox", skyboxIBLSettings, glm::vec3(0.f), std::numeric_limits<float>::infinity());
	sceneRegistry.Register(skyboxEntity);

	IBLSettings probeIBLSettings = ProbeLibrary::GetSettings("resources/textures/eqr_maps/newport_loft.hdr");
	Entity probeEntity = WorldObjectFactory::CreateEnvironmentProbe(entityManager, probeManager, idManager, "probe", probeIBLSettings, glm::vec3(0.f), 50.0f);
	sceneRegistry.Register(probeEntity);

	// Systems
	RenderSystem renderSystem;
	ProbeSystem probeSystem;

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
	PropertiesWindow propertiesWindow(&transformManager, &shaderManager, &assetManager, &materialsGroupManager, &probeManager);

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
			tex_curr = compositeScene.id;
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

			ImGui::SetCursorScreenPos({ pos.x + offset_x, pos.y + offset_y });

			ImGui::Image(tex_curr, { display_width, display_height }, { 0,1 }, { 1,0 }, ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));

			bool imageHovered = ImGui::IsItemHovered();
			if (imageHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				gViewportCaptured = true;
			}
			if (!imageHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				gViewportCaptured = false;
			}
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
		renderSystem.RenderGeometry(
			sceneRegistry, 
			transformManager, 
			shaderManager, 
			assetManager, 
			materialsGroupManager, 
			camera);
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
			probeSystem.RebuildProbes(sceneRegistry, probeManager);

			std::vector<Entity> activeProbes = 
				probeSystem.GetActiveProbes(
					sceneRegistry, 
					probeManager, 
					camera);
			std::vector<EnvironmentProbeComponent*> IBLProbes;

			for (auto& p : activeProbes) IBLProbes.push_back(probeManager.GetComponent(p));

			renderSystem.RenderDeferredPBR(
				pbrBufferShader, 
				lightPos, 
				gAttachments, 
				IBLProbes, 
				camera, 
				frameVAO);
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

			// Composite output
			compositeBuffer.bind();
			compositeShader.use();
			glm::mat4 projection = camera.getProjectionMatrix(W_WIDTH, W_HEIGHT, 0.1f, 1000.0f);
			glm::mat4 view = camera.getViewMatrix();
			glm::mat4 viewNoTrans = glm::mat4(glm::mat3(view));
			glm::mat4 invProjection = glm::inverse(projection);
			glm::mat4 invView = glm::inverse(viewNoTrans);

			compositeShader.setInt("tonemappedScene", 0);
			compositeShader.setInt("sceneDepth", 1);
			compositeShader.setInt("skybox", 2);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tonemappedScene.id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gDepth.id);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_CUBE_MAP, IBLProbes[IBLProbes.size() - 1]->maps.envMap);
			compositeShader.setMat4("invProjection", invProjection);
			compositeShader.setMat4("invView", invView);
			glBindVertexArray(frameVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			compositeBuffer.unbind();

			// Post processing
			postprocessBuffer.bind();
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
			glActiveTexture(GL_TEXTURE9);
			glBindTexture(GL_TEXTURE_2D, compositeScene.id);
			glBindVertexArray(frameVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			postprocessBuffer.unbind();
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

void processInput(GLFWwindow* window)
{
	bool escPressed = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
	static bool escPressedLastFrame = false;

	if (escPressed && !escPressedLastFrame)
	{
		if (gViewportCaptured)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			gViewportCaptured = false;
		}
		else glfwSetWindowShouldClose(window, true);
	}
	escPressedLastFrame = escPressed;

	Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	if (!camera) return;

	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	const float cameraSpeed = 12.5f * deltaTime; // Adjust as needed.

	// Update camera position based on key input:
	if (gViewportCaptured)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			camera->setCameraPos(camera->getCameraPos() + cameraSpeed * camera->getCameraFront());
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			camera->setCameraPos(camera->getCameraPos() - cameraSpeed * camera->getCameraFront());
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			// Calculate the right vector: normalized cross product of front and up.
			glm::vec3 right = glm::normalize(glm::cross(camera->getCameraFront(), camera->getCameraUp()));
			camera->setCameraPos(camera->getCameraPos() - right * cameraSpeed);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			glm::vec3 right = glm::normalize(glm::cross(camera->getCameraFront(), camera->getCameraUp()));
			camera->setCameraPos(camera->getCameraPos() + right * cameraSpeed);
		}
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!gViewportCaptured) return;

	Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	if (!camera) return;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	camera->setCameraFront(glm::normalize(direction));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	if (!camera) return;

	zoom -= (float)yoffset;
	if (zoom < 1.0f) zoom = 1.0f;
	if (zoom > 45.0f) zoom = 45.0f;

	camera->setFOV(zoom);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
