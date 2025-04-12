#pragma once
#include "texture.h"

class TextureLibrary
{
public:
	static void Register(const std::string& key, unsigned int glID, int w, int h)
	{
		GetLibrary()[key] = Texture(glID, w, h);
	}

	static Texture& GetTexture(const std::string& key)
	{
		auto it = GetLibrary().find(key);
		if (it != GetLibrary().end())
			return it->second;
		else
			throw std::runtime_error("Texture not found: " + key);
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
	static std::unordered_map<std::string, Texture>& GetLibrary()
	{
		static std::unordered_map<std::string, Texture> library;
		return library;
	}
};