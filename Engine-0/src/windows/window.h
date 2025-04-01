#pragma once
#include "imgui.h"
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

class PropertiesWindow : public Window
{
public:
	PropertiesWindow() : Window("Properties", true, ImGuiWindowFlags_NoCollapse) { }

	bool BeginRender() override 
	{
		if (!window_open) return false;

		bool renderContent = (ImGui::Begin(title.c_str(), &window_open, window_flags));
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

		bool renderContent = (ImGui::Begin(title.c_str(), &window_open, window_flags));
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