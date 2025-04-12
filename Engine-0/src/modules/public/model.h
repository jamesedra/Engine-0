#pragma once
// #include "../common.h"
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader.h"
#include "mesh.h"
#include "utils.h"
#include "texture_library.h"

// Lightweight metadata for textures gathered from MTL
struct TextureMetadata
{
	std::string path;
	std::string type;
	unsigned int id;
	int width, height;
};


class Model {
public:
	Model(const char* path) {
		loadModel(path);
	}
	void Draw(Shader& shader);
	void DrawInstanced(Shader& shader, unsigned int count);

	const std::vector<Mesh>& getMeshes() const; // may be temporary for getting mesh array
private:
	std::vector<Mesh> meshes;
	std::vector<MeshTexture> textures_loaded; // to be deprecated
	std::vector<std::string> rtextures_loaded;
	std::string directory;

	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	// to be deprecated
	std::vector<MeshTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	unsigned int TextureFromFile(const char* path, const std::string& directory, int& width, int& height, TextureColorSpace space = TextureColorSpace::Linear);
	std::vector<TextureMetadata> rloadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);

};