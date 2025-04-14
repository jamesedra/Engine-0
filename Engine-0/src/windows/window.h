#pragma once
#include "imgui.h"
#include "../modules/public/component_manager.h"
#include "../modules/public/shader_library.h"
#include "../modules/public/texture_library.h"
#include "../modules/public/asset_library.h"
#include <iostream>
#include <string>
#include <map>

class Window
{
public:
	std::string title;
	bool window_open;
	ImGuiWindowFlags window_flags;
	
	Window(const std::string& title, bool window_open, ImGuiWindowFlags window_flags = 0) : title(title), window_open(window_open), window_flags(window_flags) {}

	virtual bool BeginRender() = 0;
	virtual void EndRender() = 0;
};

class MainDockWindow : public Window
{
public:
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	MainDockWindow() : Window("##MainWindow", true)
	{
		window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	bool BeginRender() override
	{
		if (!window_open) return false;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGui::Begin(title.c_str(), &window_open, window_flags);
		
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save", NULL, false)) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Window"))
			{

				if (ImGui::MenuItem("Properties", NULL, false)) {}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		// Toolbar buttons
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 6));

		if (ImGui::Button("Save")) {  }  ImGui::SameLine();
		if (ImGui::Button("Add Actor")) {  }  ImGui::SameLine();
		if (ImGui::Button("Play")) {  }

		ImGui::PopStyleVar(2);
		ImGui::Separator();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		else
		{
			std::cout << "Docking not enabled" << std::endl;
		}

		ImGui::PopStyleVar(3);
		return true;
	}

	void EndRender() override
	{
		ImGui::End();
	}
};

class OutlinerWindow : public Window
{
private:
	SceneEntityRegistry* sceneRegistry = nullptr;
	IDManager* idManager = nullptr;
	Entity selectedEntity = INT16_MAX;
public:
	OutlinerWindow() : Window("Outliner", true, ImGuiWindowFlags_NoCollapse) { }
	OutlinerWindow(SceneEntityRegistry* sceneRegistry, IDManager* idManager) : 
		Window("Outliner", true, ImGuiWindowFlags_NoCollapse), sceneRegistry(sceneRegistry), idManager(idManager) { }

	bool BeginRender() override
	{
		if (!window_open) return false;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		bool renderContent = (ImGui::Begin(title.c_str(), &window_open, window_flags));
		
		if (renderContent && sceneRegistry && idManager)
		{
			// prepare default window
			ImVec2 region = ImGui::GetContentRegionAvail();
			if (ImGui::BeginListBox("##Outliner", region))
			{
				for (Entity entity : sceneRegistry->GetAll())
				{
					bool isSelected = entity == selectedEntity;

					std::string label = idManager->components[entity].ID + "##" + std::to_string(entity);
					if (ImGui::Selectable(label.c_str(), isSelected)) selectedEntity = entity;
					if (isSelected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();
			}
		}
		ImGui::PopStyleVar(2);
		return true;
	}

	void EndRender() override
	{
		ImGui::End();
	}

	Entity GetSelectedEntity() const
	{
		return selectedEntity;
	}

	// When other windows/elements can trigger the selected entity
	void SetSelectedEntity(Entity e)
	{
		selectedEntity = e;
	}
};

class PropertiesWindow : public Window
{
private:
	TransformManager* transformManager = nullptr;
	ShaderManager* shaderManager = nullptr;
	AssetManager* assetManager = nullptr;
	MaterialsGroupManager* materialsGroupManager = nullptr;

	// Entity to display
	Entity expandedEntity = INT16_MAX;

public:
	PropertiesWindow() : Window("Properties", true, ImGuiWindowFlags_NoCollapse) { }
	PropertiesWindow(
		TransformManager* transformManager,
		ShaderManager* shaderManager,
		AssetManager* assetManager,
		MaterialsGroupManager* materialsGroupManager)
		:
		Window("Properties", true, ImGuiWindowFlags_NoCollapse),
		transformManager(transformManager),
		shaderManager(shaderManager),
		assetManager(assetManager),
		materialsGroupManager(materialsGroupManager){ }

	bool BeginRender() override
	{
		if (!window_open) return false;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		bool renderContent = (ImGui::Begin(title.c_str(), &window_open, window_flags));
		
		if (renderContent && transformManager && shaderManager && materialsGroupManager && assetManager)
		{
			// prepare default window
			// Show transform values
			TransformComponent* transformComp = transformManager->GetComponent(expandedEntity);
			if (transformComp)
			{
				float position[4] = { transformComp->position.x, transformComp->position.y, transformComp->position.z, 1.0f };
				float rotation[4] = { transformComp->rotation.x, transformComp->rotation.y, transformComp->rotation.z, 1.0f };
				float scale[4] = { transformComp->scale.x, transformComp->scale.y, transformComp->scale.z, 1.0f };

				std::string posLabel = "Position##ExpandedPropertiesWindow";
				ImGui::DragFloat3(posLabel.c_str(), position, 0.5f);
				transformComp->position = glm::vec3(position[0], position[1], position[2]);

				std::string rotLabel = "Rotation##ExpandedPropertiesWindow";
				ImGui::DragFloat3(rotLabel.c_str(), rotation, 0.5f);
				transformComp->rotation = glm::vec3(rotation[0], rotation[1], rotation[2]);

				std::string scaleLabel = "Scale##ExpandedPropertiesWindow";
				ImGui::DragFloat3(scaleLabel.c_str(), scale, 0.5f);
				transformComp->scale = glm::vec3(scale[0], scale[1], scale[2]);
			}

			ShaderComponent* shaderComp = shaderManager->GetComponent(expandedEntity);
			if (shaderComp)
			{
				std::string shaderName = shaderComp->shaderName;
				std::vector<const char*> libShaders = ShaderLibrary::GetLibraryKeys();
				auto shader_selected = std::find_if(libShaders.begin(), libShaders.end(), [&shaderName](const char* s)
					{
						return shaderName == s;
					});
				size_t shader_index = (shader_selected != libShaders.end()) ? std::distance(libShaders.begin(), shader_selected) : 0;
				std::string shaderComboLabel = "Material##DropDownPropertiesWindow";

				if (ImGui::BeginCombo(shaderComboLabel.c_str(), shaderName.c_str(), 0))
				{
					static ImGuiTextFilter filter;
					if (ImGui::IsWindowAppearing())
					{
						ImGui::SetKeyboardFocusHere();
						filter.Clear();
					}
					ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
					std::string filterLabel = "##Filter" + std::to_string(expandedEntity);
					filter.Draw(filterLabel.c_str(), -FLT_MIN);

					for (int n = 0; n < libShaders.size(); n++)
					{
						const bool is_selected = (shader_index == n);
						if (filter.PassFilter(libShaders[n]))
						{
							if (ImGui::Selectable(libShaders[n], is_selected))
							{
								if (shaderComp->shaderName != libShaders[n])
								{
									shader_index = n;
									shaderComp->shaderName = libShaders[n];
									shaderComp->shader = &ShaderLibrary::GetShader(shaderComp->shaderName);
								}
							}
						}
					}
					ImGui::EndCombo();
				}
			}

			AssetComponent* assetComp = assetManager->GetComponent(expandedEntity);

			MaterialsGroupComponent* materialsGroupComp = materialsGroupManager->GetComponent(expandedEntity);
			if (materialsGroupComp)
			{
				auto& materialsGroup = materialsGroupComp->materialsGroup;
				for (unsigned int i = 0; i < materialsGroup.size(); i++)
				{
					std::string header = assetComp->assetName + " submesh " + std::to_string(i);
					if (ImGui::TreeNode(header.c_str()))
					{
						auto& uniforms = materialsGroup[i].material.uniforms;
						for (auto& pair : uniforms)
						{
							std::string uniformName = pair.first;
							UniformValue& uniformValue = pair.second;

							std::string uniformLabel = uniformName + "##PropertiesWindow";

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
								case UniformValue::Type::Sampler2D:
									std::string path = uniformValue.texturePath;
									std::vector<const char*> libTextures = TextureLibrary::GetLibraryKeys();
									auto tex_selected = std::find_if(libTextures.begin(), libTextures.end(), [&path](const char* s)
										{
											return path == s;
										});
									size_t tex_index = (tex_selected != libTextures.end()) ? std::distance(libTextures.begin(), tex_selected) : 0;

									std::string tex2DComboLabel = uniformName + "##Texture2DDropdownPropertiesWindow";
									if (ImGui::BeginCombo(tex2DComboLabel.c_str(), path.c_str(), 0))
									{
										static ImGuiTextFilter filter;
										if (ImGui::IsWindowAppearing())
										{
											ImGui::SetKeyboardFocusHere();
											filter.Clear();
										}
										ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
										std::string filterLabel = "##FilterPropertiesWindow:" + uniformName;
										filter.Draw(filterLabel.c_str(), -FLT_MIN);

										for (int n = 0; n < libTextures.size(); n++)
										{
											const bool is_selected = (tex_index == n);
											if (filter.PassFilter(libTextures[n]))
											{
												if (ImGui::Selectable(libTextures[n], is_selected))
												{
													if (path != libTextures[n])
													{
														tex_index = n;
														uniformValue.texturePath = libTextures[n];
													}
												}
											}
										}
										ImGui::EndCombo();
									}
									break;
							}
						}
						ImGui::TreePop();
					}
				}
			}

			// Show mesh values
			if (assetComp)
			{
				std::string assetName = assetComp->assetName;
				std::vector<const char*> libAssets = AssetLibrary::GetLibraryKeys();
				auto asset_selected = std::find_if(libAssets.begin(), libAssets.end(), [&assetName](const char* a) { return assetName == a; });
				size_t asset_index = (asset_selected != libAssets.end()) ? std::distance(libAssets.begin(), asset_selected) : 0;

				std::string assetComboLabel = "Mesh##AssetDropDownPropetiesWindow";
				if (ImGui::BeginCombo(assetComboLabel.c_str(), assetName.c_str(), 0))
				{
					static ImGuiTextFilter filter;
					if (ImGui::IsWindowAppearing())
					{
						ImGui::SetKeyboardFocusHere();
						filter.Clear();
					}
					ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
					std::string filterLabel = "##AssetFilterPropertiesWindow";
					filter.Draw(filterLabel.c_str(), -FLT_MIN);

					for (int n = 0; n < libAssets.size(); n++)
					{
						const bool is_selected = (asset_index == n);
						if (filter.PassFilter(libAssets[n]))
						{
							if (ImGui::Selectable(libAssets[n], is_selected))
							{
								if (assetComp->assetName != libAssets[n])
								{
									asset_index = n;
									assetComp->assetName = libAssets[n];

									Asset& asset = AssetLibrary::GetAsset(libAssets[n]);
									
									MaterialsGroupComponent materialsGroupComp;

									using TexturePaths = std::vector<std::string>;
									std::map<TexturePaths, std::vector<unsigned int>> textureIndexMap;
									for (unsigned int i = 0; i < asset.parts.size(); i++)
									{
										TexturePaths texturePaths;
										for (auto& textureMetaData : asset.parts[i].textures)
										{
											texturePaths.push_back(textureMetaData.path);
										}
										textureIndexMap[texturePaths].push_back(i);
									}

									materialsGroupComp.materialsGroup.reserve(textureIndexMap.size());
									for (auto& [paths, indices] : textureIndexMap)
									{
										std::vector<TextureMetadata> textures = asset.parts[indices[0]].textures;
										Material material(*shaderComp->shader, textures);
										MaterialsGroup materialsGroup{
											material, std::move(indices)
										};
										materialsGroupComp.materialsGroup.push_back(materialsGroup);
									}

									materialsGroupManager->components[expandedEntity] = std::move(materialsGroupComp);
								}
							}
						}
					}
					ImGui::EndCombo();
				}

			}
		}
		ImGui::PopStyleVar(2);
		return true;
	}

	void EndRender() override
	{
		ImGui::End();
	}

	Entity GetExpandedEntity() const
	{
		return expandedEntity;
	}

	void SetExpandedEntity(Entity e)
	{
		expandedEntity = e;
	}
};

class ViewportWindow : public Window
{
public:
	ViewportWindow() : Window("Viewport", true, ImGuiWindowFlags_NoCollapse) {}
	bool BeginRender() override
	{
		if (!window_open) return false;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		bool renderContent = (ImGui::Begin(title.c_str(), &window_open, window_flags));
		ImGui::PopStyleVar(2);
		if (renderContent)
		{
			// prepare default window
		}
		return true;

	}

	void EndRender() override
	{
		ImGui::End();
	}
};