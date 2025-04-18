#pragma once
#include "shader.h"
#include <unordered_map>

class ShaderLibrary
{
public:
	static Shader& GetShader(const std::string& name)
	{
		if (GetLibrary().empty())
			InitializeLibrary();
		auto it = GetLibrary().find(name);
		if (it != GetLibrary().end())
			return it->second;
		else
			throw std::runtime_error("Shader not found: " + name);
	}

	static std::vector<const char*> GetLibraryKeys()
	{
		auto& lib = GetLibrary();
		std::vector<const char*> keys;
		keys.reserve(lib.size());
		for (const auto& pair : lib)
		{
			keys.push_back(pair.first.c_str());
		}
		return keys;
	}

private:
	static std::unordered_map<std::string, Shader>& GetLibrary()
	{
		static std::unordered_map<std::string, Shader> library;
		return library;
	}

	static void InitializeLibrary()
	{
		Shader defaultGShader = Shader("shaders/gbuffer/gbuffer_default.vert", "shaders/gbuffer/gbuffer_default.frag");
		GetLibrary().emplace("Default Lit", std::move(defaultGShader));
		Shader PBRGShader = Shader("shaders/gbuffer/gbuffer_default.vert", "shaders/gbuffer/gbuffer_pbr.frag");
		GetLibrary().emplace("PBR Test", std::move(PBRGShader));
		Shader tintedGShader = Shader("shaders/gbuffer/gbuffer_default.vert", "shaders/gbuffer/gbuffer_tint.frag");
		GetLibrary().emplace("Lit with Color Tint", std::move(tintedGShader));
		Shader SkyboxShader = Shader("shaders/skybox/skybox_default.vert", "shaders/skybox/skybox_default.frag");
		GetLibrary().emplace("Default Skybox", std::move(SkyboxShader));
	}
};