#pragma once
#include "texture.h"
#include "loaders.h"

class TextureLibrary
{
public:
	static void Register(const std::string& key, unsigned int glID, int w, int h)
	{
		auto& lib = GetLibrary();
		if (lib.empty())
			InitializeLibrary();
		if (lib.find(key) == lib.end())
		{
			lib.emplace(key, Texture{ glID, w, h });
		}
	}

	static Texture& GetTexture(const std::string& key)
	{
		if (GetLibrary().empty())
			InitializeLibrary();
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

	static void InitializeLibrary()
	{
		Texture whiteTexture = TextureLoader::CreateWhiteTexture();
		Texture blackTexture = TextureLoader::CreateBlackTexture();
		Texture normalTexture = TextureLoader::CreateNormalTexture();
		Texture importedTexture = TextureLoader::CreateTextureFromImport("resources/textures/brickwall.jpg");

		Texture rockAlbedo = TextureLoader::CreateTextureFromImport("resources/textures/pbr/rock/albedo.png");
		Texture rockNormal = TextureLoader::CreateTextureFromImport("resources/textures/pbr/rock/normal.png");
		Texture rockRoughness = TextureLoader::CreateTextureFromImport("resources/textures/pbr/rock/roughness.png");
		Texture rockAO = TextureLoader::CreateTextureFromImport("resources/textures/pbr/rock/ao.png");
		Texture rockDisplacement = TextureLoader::CreateTextureFromImport("resources/textures/pbr/rock/displacement.png");

		Texture grassAlbedo = TextureLoader::CreateTextureFromImport("resources/textures/pbr/grass1/albedo.png");
		Texture grassNormal = TextureLoader::CreateTextureFromImport("resources/textures/pbr/grass1/normal.png");
		Texture grassRoughness = TextureLoader::CreateTextureFromImport("resources/textures/pbr/grass1/roughness.png");
		Texture grassAO = TextureLoader::CreateTextureFromImport("resources/textures/pbr/grass1/ao.png");
		Texture grassDisplacement = TextureLoader::CreateTextureFromImport("resources/textures/pbr/grass1/displacement.png");

		GetLibrary().emplace("White Texture - Default", std::move(whiteTexture));
		GetLibrary().emplace("Black Texture - Default", std::move(blackTexture));
		GetLibrary().emplace("Normal Texture - Default", std::move(normalTexture));
		GetLibrary().emplace("Imported Texture - Sample", std::move(importedTexture));

		GetLibrary().emplace("Rock Albedo", std::move(rockAlbedo));
		GetLibrary().emplace("Rock Normal", std::move(rockNormal));
		GetLibrary().emplace("Rock Roughness", std::move(rockRoughness));
		GetLibrary().emplace("Rock AO", std::move(rockAO));
		GetLibrary().emplace("Rock Displacement", std::move(rockDisplacement));

		GetLibrary().emplace("Grass Albedo", std::move(grassAlbedo));
		GetLibrary().emplace("Grass Normal", std::move(grassNormal));
		GetLibrary().emplace("Grass Roughness", std::move(grassRoughness));
		GetLibrary().emplace("Grass AO", std::move(grassAO));
		GetLibrary().emplace("Grass Displacement", std::move(grassDisplacement));
	}
};