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
private:
	static std::unordered_map<std::string, Shader>& GetLibrary()
	{
		static std::unordered_map<std::string, Shader> library;
		return library;
	}

	static void InitializeLibrary()
	{
		Shader defaultGShader = Shader("shaders/gbuffer/gbuffer.vert", "shaders/gbuffer/gbuffer.frag");
		GetLibrary().emplace("Default", std::move(defaultGShader));
	}
};