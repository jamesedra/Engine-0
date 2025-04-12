#pragma once
#include "mesh.h"
#include "texture_metadata.h"

struct MeshData
{
	Mesh mesh;
	std::vector<TextureMetadata> textures;
};