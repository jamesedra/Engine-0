#pragma once
#include "imgui.h"
#include "../modules/component_manager.h"

#include <iostream>
#include <string>

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
		ImGui::PopStyleVar(2);

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
	Entity selectedEntity = 9999999999;
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
	void   SetSelectedEntity(Entity e)
	{
		selectedEntity = e;
	}
};

class PropertiesWindow : public Window
{
public:
	PropertiesWindow() : Window("Properties", true, ImGuiWindowFlags_NoCollapse) { }
	
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