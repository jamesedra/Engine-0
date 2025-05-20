#pragma once
#include "terrain.h"
#include "terrain_lod_manager.h"

class GeomipTerrain : public Terrain
{
public:
	void GenerateGeomip(int patchSize, int worldScale = 1.0f);
	void Initialize();
	void Render(Shader& shader, Camera& camera) override;

private:
	LODManager lodManager;	// decides the LOD per patch (collection of triangle fans)
	int patchSize = 0;		// vertices per side in one patch (must be odd, 5 vertices would have 4x4 triangle fans)
	int maxLOD = 0;

	struct SingleLODInfo
	{
		int start = 0;		// start from the index buffer
		int count = 0;		// number of indices
	};

	#define LEFT 2
	#define RIGHT 2
	#define TOP 2
	#define BOTTOM 2
	struct LODInfo
	{
		SingleLODInfo info[LEFT][RIGHT][TOP][BOTTOM];
	};
	std::vector<LODInfo> lodInfo; // each element in lodInfo holds 16 permutations for the core LOD

	// patch grid dimensions
	int numPatchesX = 0;
	int numPatchesZ = 0;

	int CalcNumIndices();
	void InitIndicesData(); // index buffer helper for geomipmap
	int InitIndicesLOD(int index, int lod);
	int InitIndicesSingleLOD(int index, int lod, int l, int r, int t, int b);
	uint32_t CreateTriangleFan(int index, int lod, int l, int r, int t, int b, int x, int z);
	int AddTriangle(int index, int indexC, int index1, int index2);
};