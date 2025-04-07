#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "common.h"

// #include "modules/model.h"
#include "modules/utils.h"
#include "modules/shader.h"
#include "modules/camera.h"
#include "modules/framebuffer.h"
#include "modules/uniformbuffer.h"
#include "modules/light_types.h"
#include "modules/texture.h"

#include "windows/deprecated-window.h"
#include "windows/window.h"

#include "modules/rendersystem.h"
#include "modules/loaders.h"
#include "modules/shader_uniform.h"
#include "modules/factory.h"

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
	// specular color buffer
	Texture gAlbedoSpec(W_WIDTH, W_HEIGHT, GL_RGBA, GL_RGBA);
	gAlbedoSpec.setTexFilter(GL_NEAREST);
	gBuffer.attachTexture2D(gAlbedoSpec, GL_COLOR_ATTACHMENT2);
	// texture and renderbuffer attachments
	gBuffer.bind();
	unsigned int gbuffer_attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, gbuffer_attachments);
	gBuffer.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

	// Lit Frame buffer
	Framebuffer litBuffer(W_WIDTH, W_HEIGHT);
	// lit output
	Texture litBufferOut(W_WIDTH, W_HEIGHT, GL_RGBA, GL_RGBA);
	litBufferOut.setTexFilter(GL_NEAREST);
	litBuffer.attachTexture2D(litBufferOut, GL_COLOR_ATTACHMENT0);
	litBuffer.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

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
	
	debugGBuffer.bind();
	unsigned int debugbuffer_attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, debugbuffer_attachments);
	debugGBuffer.attachRenderbuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

	unsigned int tex_diff = loadTexture("resources/textures/brickwall.jpg", true, TextureColorSpace::sRGB);
	unsigned int tex_spec = createDefaultTexture();

	unsigned int cubeVAO = createCubeVAO();
	unsigned int frameVAO = createFrameVAO();

	Shader outShader("shaders/default.vert", "shaders/default.frag");
	Shader outputFrame("shaders/frame_out.vert", "shaders/frame_out.frag");
	//Shader gBufferShader("shaders/gbuffer/gbuffer.vert", "shaders/gbuffer/gbuffer.frag");
	Shader debugBufferShader("shaders/gbuffer/gbuffer_debug_out.vert", "shaders/gbuffer/gbuffer_debug_out.frag");
	Shader litBufferShader("shaders/NPR/npr_def.vert", "shaders/NPR/blinn_shading.frag");

	EntityManager entityManager;
	SceneEntityRegistry sceneRegistry;
	TransformManager transformManager;
	MeshManager meshManager;
	ShaderManager shaderManager;
	MaterialManager materialManager;

	Entity entityTest = WorldObjectFactory::CreateWorldMesh(entityManager, transformManager, meshManager, shaderManager, materialManager);
	sceneRegistry.Register(entityTest);

	Entity another = WorldObjectFactory::CreateWorldMesh(entityManager, transformManager, meshManager, shaderManager, materialManager);
	transformManager.components[another].position = glm::vec3(1.5f, 0.0f, 0.0f);
	sceneRegistry.Register(another);
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
	ImGui_ImplOpenGL3_Init("#version 330");

	// Windows
	MainDockWindow mainWindow;
	PropertiesWindow propertiesWindow;
	ViewportWindow viewportWindow;

	float my_color[4] = { 1.0, 1.0, 1.0, 1.0 };
	static bool viewport_active;
	static bool properties_active;
	static ImVec4 color = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
	static int tex_type = 0;
	unsigned int tex_curr = 0;

	static float lightPos[4] = { 2.5f, 5.0f, 2.5f, 0.0f };
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
		default:
			tex_curr = litBufferOut.id;
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

		// GBuffer pass
		gBuffer.bind();
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderSystem.Render(sceneRegistry, transformManager, meshManager, shaderManager, materialManager, camera);
		gBuffer.unbind();

		// deferred shading stage
		glDisable(GL_DEPTH_TEST);

		if (tex_type > 2)
		{
			// Lit shading pass
			litBuffer.bind();
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			litBufferShader.use();
			litBufferShader.setVec3("dirLight.Position", lightPos[0], lightPos[1], lightPos[2]);
			litBufferShader.setVec3("dirLight.Color", lightColor);
			litBufferShader.setInt("gPosition", 0);
			litBufferShader.setInt("gNormal", 1);
			litBufferShader.setInt("gAlbedoSpec", 2);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition.id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal.id);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, gAlbedoSpec.id);
			glBindVertexArray(frameVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			litBuffer.unbind();
		}
		else if (tex_type <= 2)
		{
			// Debug GBuffer pass
			debugGBuffer.bind();
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			debugBufferShader.use();
			debugBufferShader.setInt("gPosition", 0);
			debugBufferShader.setInt("gNormal", 1);
			debugBufferShader.setInt("gAlbedoSpec", 2);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition.id);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal.id);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, gAlbedoSpec.id);
			glBindVertexArray(frameVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			debugGBuffer.unbind();
			
		}
		glEnable(GL_DEPTH_TEST);
		
		// Property window
		properties_active = propertiesWindow.BeginRender();
		if (properties_active)
		{
			// additional rendering
			for (Entity entity : sceneRegistry.GetAll())
			{
				auto meshIt = meshManager.components.find(entity);
				if (meshIt == meshManager.components.end())
					continue;

				std::string propertyLabel = "World Object #" + std::to_string(entity);
				if (ImGui::TreeNode(propertyLabel.c_str()))
				{
					// Transform Values
					TransformComponent* transformComp = transformManager.GetComponent(entity);

					float position[4] = { transformComp->position.x, transformComp->position.y, transformComp->position.z, 1.0f };
					float rotation[4] = { transformComp->rotation.x, transformComp->rotation.y, transformComp->rotation.z, 1.0f };
					float scale[4] = { transformComp->scale.x, transformComp->scale.y, transformComp->scale.z, 1.0f };

					std::string posLabel = "Position##" + std::to_string(entity);
					ImGui::DragFloat3(posLabel.c_str(), position, 0.5f);
					transformComp->position = glm::vec3(position[0], position[1], position[2]);

					std::string rotLabel = "Rotation##" + std::to_string(entity);
					ImGui::DragFloat3(rotLabel.c_str(), rotation, 0.5f);
					transformComp->rotation = glm::vec3(rotation[0], rotation[1], rotation[2]);

					std::string scaleLabel = "Scale##" + std::to_string(entity);
					ImGui::DragFloat3(scaleLabel.c_str(), scale, 0.5f);
					transformComp->scale = glm::vec3(scale[0], scale[1], scale[2]);

					// Material Values
					ShaderComponent* shaderComp = shaderManager.GetComponent(entity);
					std::string shaderID = shaderComp->shaderID;
					std::vector<const char*> libShaders = ShaderLibrary::GetLibraryKeys();
					auto shader_selected = std::find_if(libShaders.begin(), libShaders.end(), [&shaderID](const char* s)
						{
							return shaderID == s;
						});
					size_t shader_index = (shader_selected != libShaders.end()) ? std::distance(libShaders.begin(), shader_selected) : 0;
					std::string shaderComboLabel = "Material##DropDown" + std::to_string(entity);

					if (ImGui::BeginCombo(shaderComboLabel.c_str(), shaderID.c_str(), 0))
					{
						static ImGuiTextFilter filter;
						if (ImGui::IsWindowAppearing())
						{
							ImGui::SetKeyboardFocusHere();
							filter.Clear();
						}
						ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
						std::string filterLabel = "##Filter" + std::to_string(entity);
						filter.Draw(filterLabel.c_str(), -FLT_MIN);

						for (int n = 0; n < libShaders.size(); n++)
						{
							const bool is_selected = (shader_index == n);
							if (filter.PassFilter(libShaders[n]))
							{
								if (ImGui::Selectable(libShaders[n], is_selected))
								{
									if (shaderComp->shaderID != libShaders[n])
									{
										shader_index = n;
										shaderComp->shaderID = libShaders[n];
										shaderComp->shader = &ShaderLibrary::GetShader(shaderComp->shaderID);
									}
								}
							}
						}
						ImGui::EndCombo();
					}

					MaterialComponent* materialComp = materialManager.GetComponent(entity);

					if (materialComp)
					{
						for (auto& pair : materialComp->parameters)
						{
							std::string uniformName = pair.first;
							UniformValue& uniformValue = pair.second;

							std::string uniformLabel = uniformName + "##" + std::to_string(entity);
							float uniformVec[4];

							switch (uniformValue.type)
							{
								case UniformValue::Type::Bool:
									uniformLabel += "bool";
									ImGui::Checkbox(uniformLabel.c_str(), &uniformValue.boolValue);
									break;
								case UniformValue::Type::Int:
									uniformLabel += "int";
									ImGui::InputInt(uniformLabel.c_str(), &uniformValue.intValue);
									break;
								case UniformValue::Type::Float:
									uniformLabel += "float";
									ImGui::InputFloat(uniformLabel.c_str(), &uniformValue.floatValue);
									break;
								case UniformValue::Type::Vec2:
									uniformLabel += "vec2";
									uniformVec[0] = uniformValue.vec2Value.x;
									uniformVec[1] = uniformValue.vec2Value.y;
									uniformVec[2] = 0.0f;
									uniformVec[3] = 0.0f;
									ImGui::DragFloat2(uniformLabel.c_str(), uniformVec, 0.5f);
									uniformValue.vec2Value = glm::vec2(uniformVec[0], uniformVec[1]);
									break;
								case UniformValue::Type::Vec3:
									uniformLabel += "vec3";
									uniformVec[0] = uniformValue.vec3Value.x;
									uniformVec[1] = uniformValue.vec3Value.y;
									uniformVec[2] = uniformValue.vec3Value.z;
									uniformVec[3] = 0.0f;
									ImGui::DragFloat3(uniformLabel.c_str(), uniformVec, 0.5f);
									uniformValue.vec3Value = glm::vec3(uniformVec[0], uniformVec[1], uniformVec[2]);
									break;
								case UniformValue::Type::Vec4:
									uniformLabel += "vec4";
									uniformVec[0] = uniformValue.vec4Value.x;
									uniformVec[1] = uniformValue.vec4Value.y;
									uniformVec[2] = uniformValue.vec4Value.z;
									uniformVec[3] = uniformValue.vec4Value.w;
									ImGui::DragFloat4(uniformLabel.c_str(), uniformVec, 0.5f);
									uniformValue.vec4Value = glm::vec4(uniformVec[0], uniformVec[1], uniformVec[2], uniformVec[3]);
									break;
							}
						}
					}

					// Mesh Values
					MeshComponent* meshComp = meshManager.GetComponent(entity);
					std::string meshID = meshComp->meshID;
					std::vector<const char*> libMeshes = MeshLibrary::GetLibraryKeys();
					auto mesh_selected = std::find_if(libMeshes.begin(), libMeshes.end(), [&meshID](const char* s)
						{
							return meshID == s;
						});
					size_t mesh_index = (mesh_selected != libMeshes.end()) ? std::distance(libMeshes.begin(), mesh_selected) : 0;

					std::string meshComboLabel = "Mesh##DropDown" + std::to_string(entity);
					if (ImGui::BeginCombo(meshComboLabel.c_str(), meshID.c_str(), 0))
					{
						static ImGuiTextFilter filter;
						if (ImGui::IsWindowAppearing())
						{
							ImGui::SetKeyboardFocusHere();
							filter.Clear();
						}
						ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
						std::string filterLabel = "##Filter" + std::to_string(entity);
						filter.Draw(filterLabel.c_str(), -FLT_MIN);

						for (int n = 0; n < libMeshes.size(); n++)
						{
							const bool is_selected = (mesh_index == n);
							if (filter.PassFilter(libMeshes[n]))
							{
								if (ImGui::Selectable(libMeshes[n], is_selected))
								{
									if (meshComp->meshID != libMeshes[n])
									{
										mesh_index = n;
										meshComp->meshID = libMeshes[n];
										meshComp->mesh = &MeshLibrary::GetMesh(meshComp->meshID);
									}
								}
							}
						}
						ImGui::EndCombo();
					}
					ImGui::TreePop();
				}
			}
			//static ImGuiColorEditFlags base_flags = ImGuiColorEditFlags_None;
			//ImGui::ColorPicker4("##picker", (float*)&color, base_flags | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
			ImGui::RadioButton("World Position", &tex_type, 0);
			ImGui::RadioButton("World Normal", &tex_type, 1);
			ImGui::RadioButton("Base Color", &tex_type, 2);
			ImGui::RadioButton("Lit", &tex_type, 3);

			ImGui::DragFloat3("Light Position", lightPos, 0.5f, -50.0f, 50.0f);
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
