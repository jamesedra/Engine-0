#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "common.h"
#include "modules/public/utils.h"
#include "modules/public/camera.h"
#include "modules/public/renderer.h"
#include "modules/public/framebuffer.h"
#include "modules/public/texture.h"
#include "windows/window.h"
#include "modules/public/light_system.h"
#include "modules/public/render_system.h"
#include "modules/public/probe_system.h"
#include "modules/public/factory.h"
#include "modules/public/contexts.h"
#include "modules/public/ibl_generator.h"
#include "modules/public/terrain.h"
#include "modules/public/terrain_brute.h"
#include "modules/public/terrain_geomip.h"
#include "modules/public/terrain_tess.h"

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
unsigned int getBufferOut(Renderer& renderer, int type);

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

	Camera camera(
		glm::vec3(2.10f, 9.43f, 39.18f),
		glm::vec3(0.25f, -0.07f, -0.97f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		45.0f
	);
	glfwSetWindowUserPointer(window, &camera);

	// Viewport framebuffer (this is a test framebuffer)
	Framebuffer viewportFrame(W_WIDTH, W_HEIGHT);
	Texture viewportOutTexture(W_WIDTH, W_HEIGHT, GL_RGBA, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
	viewportFrame.attachTexture2D(viewportOutTexture, GL_COLOR_ATTACHMENT0);
	viewportFrame.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

	// Render pipeline
	Renderer renderer;
	renderer.Initialize(W_WIDTH, W_HEIGHT);

	// Object tests
	unsigned int cubeVAO = createCubeVAO();
	unsigned int frameVAO = createFrameVAO();

	// Shaders
	// Deferred Shading
	Shader outShader("shaders/frame_out.vert", "shaders/frame_out.frag");
	Shader outputFrame("shaders/frame_out.vert", "shaders/frame_out.frag");

	// -------------------
	// Component Managers
	EntityManager entityManager;
	IDManager idManager;
	TransformManager transformManager;
	ShaderManager shaderManager;
	AssetManager assetManager;
	MaterialsGroupManager materialsGroupManager;
	EnvironmentProbeManager probeManager;
	LightManager lightManager;
	LandscapeManager landscapeManager;

	// Registries
	SceneEntityRegistry sceneRegistry;

	// Contexts
	WorldContext worldContext(&entityManager, &transformManager, &shaderManager, &assetManager, &materialsGroupManager);
	OutlinerContext outlinerContext(&sceneRegistry, &idManager);
	
	// IBL
	// probe entities
	IBLSettings skyboxIBLSettings = ProbeLibrary::GetSettings("resources/textures/eqr_maps/kloofendal_43d_clear_puresky_2k.hdr");
	Entity skyboxEntity = WorldObjectFactory::CreateSkyProbe(entityManager, probeManager, idManager, "skybox", skyboxIBLSettings, glm::vec3(0.f));
	sceneRegistry.Register(skyboxEntity);

	//IBLSettings probeIBLSettings = ProbeLibrary::GetSettings("resources/textures/eqr_maps/newport_loft.hdr");
	//Entity probeEntity = WorldObjectFactory::CreateEnvironmentProbe(entityManager, probeManager, idManager, "probe", probeIBLSettings, glm::vec3(0.f), 50.0f);
	//sceneRegistry.Register(probeEntity);

	// World Objects
	//Entity floorEntity = WorldObjectFactory::CreateWorldObject(worldContext, "", "", "");
	//idManager.components[floorEntity].ID = "floor";
	//transformManager.components[floorEntity].position = glm::vec3(0.0f, -2.0f, 0.0f);
	//transformManager.components[floorEntity].scale = glm::vec3(100.0f, 0.5f, 100.0f);
	//sceneRegistry.Register(floorEntity);

	Entity cubeEntity = WorldObjectFactory::CreateWorldObject(worldContext, "", "Cube", "");
	idManager.components[cubeEntity].ID = "cube";
	transformManager.components[cubeEntity].position = glm::vec3(0.0f, 3.0f, -4.5f);
	transformManager.components[cubeEntity].rotation = glm::vec3(-0.5f, 4.0f, 0.0f);
	transformManager.components[cubeEntity].scale = glm::vec3(3.0f);
	sceneRegistry.Register(cubeEntity);

	Entity sphereEntity = WorldObjectFactory::CreateWorldObject(worldContext, "", "Sphere", "");
	idManager.components[sphereEntity].ID = "sphere";
	transformManager.components[sphereEntity].position = glm::vec3(-5.0f, 1.5f, 5.0f);
	transformManager.components[sphereEntity].scale = glm::vec3(2.0f);
	sceneRegistry.Register(sphereEntity);

	Entity sphere1Entity = WorldObjectFactory::CreateWorldObject(worldContext, "", "Sphere", "");
	idManager.components[sphere1Entity].ID = "sphere1";
	transformManager.components[sphere1Entity].position = glm::vec3(10.0f, 4.0f, -25.5f);
	transformManager.components[sphere1Entity].scale = glm::vec3(3.0f);
	sceneRegistry.Register(sphere1Entity);

	Entity coneEntity = WorldObjectFactory::CreateWorldObject(worldContext, "", "Cone", "");
	idManager.components[coneEntity].ID = "cone";
	transformManager.components[coneEntity].position = glm::vec3(25.0f, -3.5f, -13.0f);
	transformManager.components[coneEntity].rotation = glm::vec3(0.5f, -4.0f, -6.5f);
	transformManager.components[coneEntity].scale = glm::vec3(3.0f, 6.0f, 3.0f);
	sceneRegistry.Register(coneEntity);

	Entity backpackEntity = WorldObjectFactory::CreateWorldObject(worldContext, "", "", "resources/objects/backpack/backpack.obj");
	idManager.components[backpackEntity].ID = "backpack";
	sceneRegistry.Register(backpackEntity);

	Entity dirLightEntity = WorldObjectFactory::CreateDirectionalLight(entityManager, lightManager, transformManager, idManager, "sun");
	sceneRegistry.Register(dirLightEntity);

	HeightmapParams heightMap{ "resources/textures/heightmaps/terrain_sample1.png" };
	Entity landscapeEntity = WorldObjectFactory::CreateLandscape(entityManager, landscapeManager, transformManager, shaderManager, materialsGroupManager, idManager, "landscape", TerrainType::Geomipmap, heightMap, 30.0f);
	sceneRegistry.Register(landscapeEntity);

	// landscape transform override
	transformManager.GetComponent(landscapeEntity)->position = glm::vec3(-220.0f, 0.0f, -67.5f);

	// Point light Objects
	//for (int i = 0; i < 50; i++)
	//{
	//	for (int j = 0; j < 50; j++)
	//	{
	//		Entity lightEntity = WorldObjectFactory::CreatePointLight(entityManager, lightManager, transformManager, idManager, "light " + std::to_string(i) + std::to_string(j), 
	//			glm::vec3(200.0f + (i*1.8f), 4.0f, 40.0f + (j*1.8f)), glm::vec3(i / 50.0f, (i*j) / 2500.0f, j / 50.0f), 20.0f);
	//		sceneRegistry.Register(lightEntity);
	//	}
	//}

	for (int i = 0; i < 50; i++)
	{
		for (int j = 0; j < 50; j++)
		{
			Entity lightEntity = WorldObjectFactory::CreatePointLight(entityManager, lightManager, transformManager, idManager, "light " + std::to_string(i) + std::to_string(j),
				glm::vec3(-20.0f + (i * 1.8f), 4.0f, -27.5f + (j * 1.8f)), glm::vec3(i / 50.0f, (i * j) / 2500.0f, j / 50.0f), 20.0f);
			sceneRegistry.Register(lightEntity);
		}
	}
	// Entity lightEntity = WorldObjectFactory::CreatePointLight(entityManager, lightManager, transformManager, idManager, "light0", glm::vec3(0, 0.0f, 0), glm::vec3(1.0f), 50.0f);
	// sceneRegistry.Register(lightEntity);


	// Systems
	LightSystem lightSystem(W_WIDTH, W_HEIGHT);
	RenderSystem renderSystem(renderer);
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

	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
		{
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}

		// camera coords
		//std::cout << "pos x: " << camera.getCameraPos().x << std::endl;
		//std::cout << "pos y: " << camera.getCameraPos().y << std::endl;
		//std::cout << "pos z: " << camera.getCameraPos().z << std::endl;
		//std::cout << "for x: " << camera.getCameraFront().x << std::endl;
		//std::cout << "for y: " << camera.getCameraFront().y << std::endl;
		//std::cout << "for z: " << camera.getCameraFront().z << std::endl;
		//std::cout << "up x: " << camera.getCameraUp().x << std::endl;
		//std::cout << "up y: " << camera.getCameraUp().y << std::endl;
		//std::cout << "up z: " << camera.getCameraUp().z << std::endl;

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

			ImGui::Image(getBufferOut(renderer, tex_type), {display_width, display_height}, {0,1}, {1,0}, ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));

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

			propertiesWindow.EndRender();
		}

		// GBuffer pass
		renderSystem.RenderGeometry(
			sceneRegistry, 
			transformManager, 
			shaderManager, 
			assetManager, 
			landscapeManager,
			materialsGroupManager, 
			camera);

		// Shadow pass
		renderSystem.RenderShadowPass(lightManager, transformManager, sceneRegistry, assetManager, landscapeManager, camera);
		// SSAO pass
		renderSystem.RenderSSAO(camera, frameVAO);
		// deferred shading stage
		glDisable(GL_DEPTH_TEST);

		if (tex_type > 5)
		{
			renderer.BlitGToLBuffers(W_WIDTH, W_HEIGHT);

			// PBR shading
			renderer.getHDRBuffer().bind();
			EnvironmentProbeComponent* skyProbe = probeManager.GetSkyProbe();
			probeSystem.RebuildProbes(sceneRegistry, probeManager);

			std::vector<Entity> activeProbes = probeSystem.GetActiveProbes(sceneRegistry, probeManager, camera);
			std::vector<EnvironmentProbeComponent*> IBLProbes;

			for (auto& p : activeProbes) IBLProbes.push_back(probeManager.GetProbeComponent(p));

			lightSystem.TileLighting(sceneRegistry, lightManager, transformManager, camera);
			lightSystem.ConfigurePBRUniforms(renderer.getPBRShader(), sceneRegistry, lightManager, transformManager);
			renderSystem.RenderPBR(skyProbe, IBLProbes, camera, frameVAO);
			renderer.getHDRBuffer().unbind();

			// Brightness pass
			renderSystem.RenderDeferredBrightness(frameVAO);
			// Blur shading
			renderSystem.RenderBlur(frameVAO);
			// Bloom shading
			renderSystem.RenderBloom(frameVAO);
			// Tone mapping
			renderSystem.RenderTonemap(frameVAO);
			// Composite output
			renderSystem.RenderComposite(skyProbe, camera, frameVAO);
			// Post processing
			renderSystem.RenderPostProcess(frameVAO);
		}
		else if (tex_type <= 5)
		{
			renderSystem.RenderBufferPass(frameVAO);
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

	const float cameraSpeed = 100.0f * deltaTime; // Adjust as needed.

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

unsigned int getBufferOut(Renderer& renderer, int type)
{
	DebugAttachments da = renderer.getDebugAttachments();

	switch (type)
	{
	case 0:
		return da.position;
	case 1:
		return da.normal;
	case 2:
		return da.albedo;
		break;
	case 3:
		return da.metallic;
	case 4:
		return da.roughness;
	case 5:
		return da.AO;
	case 6:
		// return renderer.getShadowMoments().id;
		return renderer.getCompositeSceneTex().id;
		//return renderer.getHDRSceneTex().id;
	default:
		// return renderer.getShadowMoments().id;
		return renderer.getPPSceneTex().id;
	}
}
