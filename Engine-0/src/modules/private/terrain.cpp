#include "../public/terrain.h"

bool Terrain::LoadHeightMap(const char* filename)
{
	int width, height, nChannels;
	unsigned char* img = stbi_load(filename, &width, &height, &nChannels, STBI_grey);
	if (!img)
	{
		std::cerr << "Error: internal format not supported." << std::endl;
		return false;
	}

	// if there is a height data, unload it first
	if (!heightData.data.empty()) UnloadHeightMap();

	heightData.data.assign(img, img + width * height);
	stbi_image_free(img);

	heightData.width = width;
	heightData.depth = height;

	return true;
}

bool Terrain::SaveHeightMap(const char* filename)
{
	// TODO:
	return false;
}

bool Terrain::UnloadHeightMap()
{
	if (!heightData.data.empty()) heightData.unload();
	return true;
}
