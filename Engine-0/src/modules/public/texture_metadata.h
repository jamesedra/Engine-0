#pragma once

#include <iostream>

// Lightweight metadata for textures gathered from MTL
struct TextureMetadata
{
	std::string path;
	std::string type;
	unsigned int id;
	int width, height;
};
