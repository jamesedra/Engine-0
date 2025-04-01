#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "common.h"

#include "modules/model.h"
#include "modules/utils.h"
#include "modules/shader.h"
#include "modules/camera.h"
#include "modules/framebuffer.h"
#include "modules/uniformbuffer.h"
#include "modules/light_types.h"
#include "modules/texture.h"

#include "windows/deprecated-window.h"
#include "windows/window.h"

constexpr int W_WIDTH = 1600;
constexpr int W_HEIGHT = 1200;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(W_WIDTH, W_HEIGHT, "PBR Engine", NULL, NULL);
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

	// Camera settings
	//Camera camera(
	//	glm::vec3(8.0f, 8.0f, 8.0f),
	//	glm::vec3(-1.0f, -1.0f, -1.0f),
	//	glm::vec3(0.0f, 1.0f, 0.0f),
	//	45.0f
	//);
	//glfwSetWindowUserPointer(window, &camera);

	//Framebuffer viewportFrame(W_WIDTH, W_HEIGHT);
	//Texture viewportOutTexture(W_WIDTH, W_HEIGHT, GL_RGBA, GL_RGBA);
	//viewportOutTexture.setTexFilter(GL_NEAREST);
	//viewportOutTexture.setTexWrap(GL_CLAMP_TO_EDGE);
	//viewportFrame.attachTexture2D(viewportOutTexture, GL_COLOR_ATTACHMENT0);
	//viewportFrame.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

	//unsigned int cubeVAO = createCubeVAO();
	//unsigned int frameVAO = createFrameVAO();

	//Shader outShader("shaders/default.vert", "shaders/default.frag");
	//Shader outputFrame("shaders/frame_out.vert", "shaders/frame_out.frag");

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
	ImGui_ImplOpenGL3_Init("#version 330");

	// Windows
	MainDockWindow mainWindow;
	PropertiesWindow propertiesWindow;
	ViewportWindow viewportWindow;

	float my_color[4] = { 1.0, 1.0, 1.0, 1.0 };
	static bool viewport_active;
	static bool properties_active;

	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
		{
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}

		processInput(window);
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		mainWindow.BeginRender();
		mainWindow.EndRender();
		viewport_active = viewportWindow.BeginRender();
		if (viewport_active) 
		{ 
			// additional rendering 
			viewportWindow.EndRender();
		}
			
		

		properties_active = propertiesWindow.BeginRender();
		if (properties_active)
		{
			// additional rendering
			propertiesWindow.EndRender();
		}
			
		

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
