#pragma once

#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader.h"
#include "mesh.h"
#include "mesh_data.h"
#include "utils.h"
#include "texture_library.h"
#include "texture_metadata.h"

class Model {
public:
	Model(const char* path) {
		loadModel(path);
	}

	const std::vector<MeshData>& getMeshData() const
	{
		return meshDataList;
	}
private:
	std::vector<MeshData> meshDataList;
	std::unordered_map<std::string, TextureMetadata> texturesLoaded;
	std::string directory;

	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	MeshData processMeshData(aiMesh* mesh, const aiScene* scene);
	unsigned int TextureFromFile(const char* path, const std::string& directory, int& width, int& height, TextureColorSpace space = TextureColorSpace::Linear);
	std::vector<TextureMetadata> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
};