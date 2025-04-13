#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>

#include "mesh_data.h"
#include "model.h"

// NOTE: This header is meant to fully replace the mesh library.
// (I kept on iterating because I did not design the engine first, don't follow what I did)
// The main problem with the mesh library and component is that it's too lightweight.
// It can handle one mesh but if I were to load an .obj file with multiple meshes (and textures)
// everything won't work at all.

// An asset holds a vector of MeshData, where a MeshData has a mesh and a vector of TextureMetaData
struct Asset
{
	std::vector<MeshData> parts;
};

class AssetLibrary
{
public:

	// this uses the model loader class
	// when the asset cannot be found in the library, it loads and saves the model
	// given that the path is valid.
	static Asset& GetAsset(const std::string& name, const std::string& path = "")
	{
		if (GetLibrary().empty())
			InitializeLibrary();

		auto it = GetLibrary().find(name);

		if (it != GetLibrary().end())
			return it->second;

		if (path.empty()) 
			throw std::runtime_error("Asset" + name + " is not found and path given is invalid : " + path);

		Model model(path.c_str());
		Asset asset;

		for (auto& meshData : model.getMeshData())
		{
			asset.parts.push_back(meshData);
		}

		return GetLibrary().emplace(name, std::move(asset)).first->second;
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
	static std::unordered_map<std::string, Asset>& GetLibrary()
	{
		static std::unordered_map<std::string, Asset> library;
		return library;
	}

	static void InitializeLibrary()
	{
		Mesh cubeMesh = MeshLoader::CreateCube();
		Mesh sphereMesh = MeshLoader::CreateSphere(1.0f, 36, 18);
		Mesh coneMesh = MeshLoader::CreateCone(1.0f, 1.5f, 36, 18);

		GetLibrary().emplace("Cube", Asset{ {MeshData{cubeMesh, {}}} }); // damn
		GetLibrary().emplace("Sphere", Asset{ {MeshData{sphereMesh, {}}} });
		GetLibrary().emplace("Cone", Asset{ {MeshData{coneMesh, {}}} });

	}
};