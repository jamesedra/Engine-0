#pragma once
#include "loaders.h"
#include "mesh.h"
#include <unordered_map>

class MeshLibrary
{
public:
	static Mesh& GetMesh(const std::string& name)
	{
		if (GetLibrary().empty())
			InitializeLibrary();
		auto it = GetLibrary().find(name);
		if (it != GetLibrary().end())
			return it->second;
		else
			throw std::runtime_error("Mesh not found: " + name);
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
	static std::unordered_map<std::string, Mesh>& GetLibrary()
	{
		static std::unordered_map<std::string, Mesh> library;
		return library;
	}

	static void InitializeLibrary()
	{
		Mesh cubeMesh = MeshLoader::CreateCube();
		Mesh sphereMesh = MeshLoader::CreateSphere(1.0f, 36, 18);
		Mesh coneMesh = MeshLoader::CreateCone(1.0f, 1.5f, 36, 18);
		GetLibrary().emplace("Cube", std::move(cubeMesh));
		GetLibrary().emplace("Sphere", std::move(sphereMesh));
		GetLibrary().emplace("Cone", std::move(coneMesh));
	}
};