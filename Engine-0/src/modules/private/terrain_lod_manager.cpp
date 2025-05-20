#include "../public/terrain_lod_manager.h"

int LODManager::InitLODManager(int patchSize, int numPatchesX, int numPatchesZ, float worldScale)
{
	this->patchSize = patchSize;
	this->numPatchesX = numPatchesX;
	this->numPatchesZ = numPatchesZ;
	this->worldScale = worldScale;

	CalcMaxLOD(); // fills max LOD
	CalcLODRegions();	// builds
	
	// Assign zeroes on the 2D array
	map.assign(numPatchesZ, std::vector<PatchLOD>(numPatchesX));
	return maxLOD; // return to geomip terrain
}

/*
 * Calculates the core LOD for a patch based on the distance from campos to the patch's center
 * Matches the ring LOD of every patch to the core LOD of its neighbors
 */
void LODManager::UpdateLOD(const glm::vec3& camPos)
{
	// std::cout << "x=" << camPos.x << " y=" << camPos.y << " z=" << camPos.z << std::endl;
	UpdateLODMapPass1(camPos);
	UpdateLODMapPass2();
}

// update the core LOD for each patch
void LODManager::UpdateLODMapPass1(const glm::vec3& camPos)
{
	// offset from patch origin to center
	int centerStep = patchSize / 2;

	for (int lodMapZ = 0; lodMapZ < numPatchesZ; lodMapZ++)
	{
		for (int lodMapX = 0; lodMapX < numPatchesX; lodMapX++)
		{
			int cx = lodMapX * (patchSize - 1) + centerStep;
			int cz = lodMapZ * (patchSize - 1) + centerStep;

			// TODO: Change the distance check to match coordinates.
			// terrain patches aren't in world space
			 glm::vec3 patchCenter = glm::vec3(cx * worldScale, 0.0f, cz * worldScale);
			 float distToCam = glm::distance(camPos, patchCenter);

			// debug checking
			 // if (lodMapX == numPatchesX - 1 && lodMapZ == numPatchesZ - 1) std::cout << "patch[" << lodMapZ << "][" << lodMapX << "] distance = " << distToCam << std::endl;
			int coreLOD = DistanceToLOD(distToCam);

			map[lodMapZ][lodMapX].core = coreLOD;
		}
	}
}

// fix ring LOD for each patch
void LODManager::UpdateLODMapPass2()
{
	const auto neighborCoreLOD = [&](int x, int z)
		{
			return map[z][x].core;
		};

	for (int lodMapZ = 0; lodMapZ < numPatchesZ; lodMapZ++)
	{
		for (int lodMapX = 0; lodMapX < numPatchesX; lodMapX++)
		{
			PatchLOD& currPatch = map[lodMapZ][lodMapX];

			// set defaults
			currPatch.left = currPatch.right = currPatch.top = currPatch.bottom = 0;

			// Set left ring
			if (lodMapX > 0 && neighborCoreLOD(lodMapX - 1, lodMapZ) > currPatch.core) currPatch.left = 1;
			// Set right ring
			if (lodMapX < numPatchesX - 1 && neighborCoreLOD(lodMapX + 1, lodMapZ) > currPatch.core) currPatch.right = 1;
			// Set top ring
			if (lodMapZ < numPatchesZ - 1 && neighborCoreLOD(lodMapX, lodMapZ + 1) > currPatch.core) currPatch.top = 1;
			// Set bottom ring
			if (lodMapZ > 0 && neighborCoreLOD(lodMapX, lodMapZ - 1) > currPatch.core) currPatch.bottom = 1;
		}
	}
}

int LODManager::DistanceToLOD(float dist)
{
	for (int i = 0; i <= maxLOD; i++)
		if (dist < regions[i]) return i;
	return maxLOD;
}

// helper for the 2D vector PatchLOD
const LODManager::PatchLOD& LODManager::GetPatchLOD(int patchX, int patchZ) const
{
	return map[patchZ][patchX];
}

/*
 * Mapping LOD based on distance from camera position:
 * 
 * Option 1: divide the range evely between LODs from campos to farplane + campos
 * (campos)[<-LOD0 Range->][<-LOD1 Range->][<-LOD2 Range->]
 
 * Option 2: increments (range of LOD1 is 2x the range of LOD0, range of LOD2 is 3x the range of LOD0)
 * (campos)[<- LOD0 ->][<-   LOD1   ->][<-     LOD2     ->] 
 * 
 * Implementation is based on option 2.
 */
void LODManager::CalcLODRegions()
{
	regions.clear();
	regions.resize(maxLOD + 1);
	// TODO: change later to more dynamic far value
	#define Z_FAR 500.0f

	int sum = 0;
	for (int i = 0; i <= maxLOD; i++) sum += i + 1;

	float base = Z_FAR / (float)((maxLOD + 1) * (maxLOD + 2) / 2);

	float acc = 0.0f;
	for (int i = 0; i <= maxLOD; i++)
	{
		acc += (base * float(i + 1));
		regions[i] = acc;
	}
}

// Calculating max LOD = log_2(patchSize - 1)
void LODManager::CalcMaxLOD()
{
	// largest power-of-two step inside the patch
	// since patchSize is odd, patchSize-1 % 2 == 0.
	maxLOD = (int)(std::floor(std::log2(patchSize - 1))) - 1;

	maxLOD = std::max(maxLOD, 0);
	regions.resize(maxLOD + 1);
}