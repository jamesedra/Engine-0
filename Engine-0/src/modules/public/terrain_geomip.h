#pragma once
#include "terrain.h"

class GeomipTerrain : public Terrain
{
public:
	void GenerateGeomip(int patchSize);
	void Initialize();
	void Render(Shader& shader) override;

private:
	int patchSize = 0;
	int maxLOD = 0;

	struct SingleLODInfo
	{
		int start = 0; // start from the index buffer
		int count = 0; // number of indices
	};

	#define LEFT 2
	#define RIGHT 2
	#define TOP 2
	#define BOTTOM 2
	struct LODInfo
	{
		SingleLODInfo info[LEFT][RIGHT][TOP][BOTTOM];
	};
	std::vector<LODInfo> lodInfo;

	int numPatchesX = 0;
	int numPatchesZ = 0;
};