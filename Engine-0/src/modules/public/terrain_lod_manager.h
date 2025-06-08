#pragma once
#include "../../common.h"

// NOTE: This class is a helper to calculate the following:
// 1. Init: Calculate the maximum LOD based on the size of the patch
// 2. Update: Choose the LOD based on the camera pos for each patch (core + ring)

class LODManager
{
public:
	int InitLODManager(int patchSize, int numPatchesX, int numPatchesZ, float worldScale);
	void UpdateLOD(const glm::vec3& camPos);

	struct PatchLOD
	{
		int core = 0;
		int left = 0;
		int right = 0;
		int top = 0;
		int bottom = 0;
	};
	const PatchLOD& GetPatchLOD(int patchX, int patchZ) const;

private:
	void CalcLODRegions();
	void CalcMaxLOD();
	void UpdateLODMapPass1(const glm::vec3& camPos);
	void UpdateLODMapPass2();
	int DistanceToLOD(float dist);

	int maxLOD = 0;
	int patchSize = 0;
	int numPatchesX = 0;
	int numPatchesZ = 0;
	float worldScale = 1.0f;

	std::vector<std::vector<PatchLOD>> map;
	std::vector<int> regions;
};