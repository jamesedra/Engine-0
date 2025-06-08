#include "../public/terrain_geomip.h"

void GeomipTerrain::Initialize()
{
	GenerateGeomip(65, 1);
}

void GeomipTerrain::GenerateGeomip(int patchSize, int worldScale)
{
	int width = heightData.width;
	int depth = heightData.depth;

	// guard rails
	if (heightData.data.empty() || width == 0 || depth == 0)
	{
		std::cerr << "GeomipTerrain::GenerateGeomip called before height data was loaded\n";
		return;
	}
	if (patchSize < 3)
	{
		std::cerr << "Minimum patch size is 3. Sent patch size: " << patchSize << std::endl;
		return;
	}
	if (patchSize % 2 == 0)
	{
		std::cerr << "Patch size must be an odd number: " << patchSize << std::endl;
		return;
	}
	if ((width - 1) % (patchSize - 1) != 0)
	{
		int recommendedWidth = 
			((width - 1 + patchSize + 1) / (patchSize - 1)) * (patchSize - 1) + 1;
		std::cerr << "terrain width - 1: " << width - 1 << 
			" must be divisible by patch size - 1: " << patchSize - 1 << 
			"\ntry using width: " << recommendedWidth << std::endl;
		return;
	}
	if ((depth - 1) % (patchSize - 1) != 0)
	{
		int recommendedDepth =
			((depth - 1 + patchSize + 1) / (patchSize - 1)) * (patchSize - 1) + 1;
		std::cerr << "terrain depth - 1: " << depth - 1 <<
			" must be divisible by patch size - 1: " << patchSize - 1 <<
			"\ntry using depth: " << recommendedDepth << std::endl;
		return;
	}

	this->patchSize = patchSize;
	this->numPatchesX = (width - 1) / (patchSize - 1);
	this->numPatchesZ = (depth - 1) / (patchSize - 1);

	SetWorldScale(worldScale);
	this->maxLOD = lodManager.InitLODManager(patchSize, numPatchesX, numPatchesZ, worldScale);
	lodInfo.resize(maxLOD + 1);

	InitBuffers();
}

void GeomipTerrain::InitBuffers()
{
	// vertices
	InitHeightVertexData();
	// indices
	indices.clear();
	indices.reserve(CalcNumIndices());
	InitIndicesData();
	PopulateBufferData();
}

// NOTE: Does not output the exact indices count for the buffer. This provides an estimate of the worst-case upper bound space.
int GeomipTerrain::CalcNumIndices()
{
	int numQuads = (patchSize - 1) * (patchSize - 1);
	int numIndices = 0;
	int maxPermutationsPerLevel = 16;	// true/false for each side
	const int indicesPerQuad = 6;		// two triangles

	for (int lod = 0; lod <= maxLOD; lod++)
	{
		std::cout << "LOD " << lod << 
			": num quads " << numQuads << std::endl;
		numIndices += numQuads * indicesPerQuad * maxPermutationsPerLevel;
		numQuads /= 4;
	}
	std::cout << "Initial number of indices: " << numIndices << std::endl;

	return numIndices;
}

// Initializes index data for each LOD
void GeomipTerrain::InitIndicesData()
{
	int index = 0;
	for (int lod = 0; lod <= maxLOD; lod++) index = InitIndicesLOD(index, lod);
}

int GeomipTerrain::InitIndicesLOD(int index, int lod)
{
	int totalIndicesForLOD = 0;

	// traverse all 16 permutations
	for (int l = 0; l < LEFT; l++)
	{
		for (int r = 0; r < RIGHT; r++)
		{
			for (int t = 0; t < TOP; t++)
			{
				for (int b = 0; b < BOTTOM; b++)
				{
					lodInfo[lod].info[l][r][t][b].start = index;
					index = InitIndicesSingleLOD(index, lod, lod + l, lod + r, lod + t, lod + b);

					lodInfo[lod].info[l][r][t][b].count = index - lodInfo[lod].info[l][r][t][b].start;
					totalIndicesForLOD += lodInfo[lod].info[l][r][t][b].count;
				}
			}
		}
	}
	std::cout << "Total indices for LOD " << lod << ": " << totalIndicesForLOD << std::endl;
	return index;
}

// Iterates all centers of the patch
int GeomipTerrain::InitIndicesSingleLOD(int index, int lod, int l, int r, int t, int b)
{
	// how many vertices before getting to the next triangle fan
	int fanStep = static_cast<int>(pow(2, lod + 1)); // LOD = 0 -> 2, 1 -> 4, 2 -> 8
	int endPos = patchSize - 1 - fanStep; // patch size 5, fanstep 2 -> endPos = 2, patch size 9, fan step 2 -> endPos 6

	for (int z = 0; z <= endPos; z+=fanStep)
	{
		for (int x = 0; x <= endPos; x+=fanStep)
		{
			// checks if the position is the end of the fan. Copies the side lod if so.
			int left = x == 0 ? l : lod;
			int right = x == endPos ? r : lod;
			int top = z == endPos ? t : lod;
			int bottom = z == 0 ? b : lod;

			index = CreateTriangleFan(index, lod, left, right, top, bottom, x, z);
		}
	}
	return index;
}

// Adds proper triangle fans (up to 8 triangles) into the indices buffer data
uint32_t GeomipTerrain::CreateTriangleFan(int index, int lod, int l, int r, int t, int b, int x, int z)
{
	int stepL = static_cast<int>(pow(2, l));
	int stepR = static_cast<int>(pow(2, r));
	int stepT = static_cast<int>(pow(2, t));
	int stepB = static_cast<int>(pow(2, b));
	int stepC = static_cast<int>(pow(2, lod));

	int w = heightData.width;

	uint32_t indexC = (z + stepC) * w + x + stepC;

	// first up
	uint32_t indexTemp1 = z * w + x;
	uint32_t indexTemp2 = (z + stepL) * w + x;
	index = AddTriangle(index, indexC, indexTemp1, indexTemp2);

	// second up
	if (l == lod)
	{
		indexTemp1 = indexTemp2;
		indexTemp2 += stepL * w;
		index = AddTriangle(index, indexC, indexTemp1, indexTemp2);
	}

	// first right
	indexTemp1 = indexTemp2;
	indexTemp2 += stepT;
	index = AddTriangle(index, indexC, indexTemp1, indexTemp2);

	// second right
	if (t == lod) // create another triangle if top lod == core lod, skipped when t > lod
	{
		indexTemp1 = indexTemp2;
		indexTemp2 += stepT;
		index = AddTriangle(index, indexC, indexTemp1, indexTemp2);
	}

	// first down
	indexTemp1 = indexTemp2;
	indexTemp2 -= stepR * w;
	index = AddTriangle(index, indexC, indexTemp1, indexTemp2);

	// second down
	if (r == lod)
	{
		indexTemp1 = indexTemp2;
		indexTemp2 -= stepR * w;
		index = AddTriangle(index, indexC, indexTemp1, indexTemp2);
	}

	// first left
	indexTemp1 = indexTemp2;
	indexTemp2 -= stepB;
	index = AddTriangle(index, indexC, indexTemp1, indexTemp2);

	// second left
	if (b == lod)
	{
		indexTemp1 = indexTemp2;
		indexTemp2 -= stepB;
		index = AddTriangle(index, indexC, indexTemp1, indexTemp2);
	}

	return index;
}

// helper to populate the indices vector
int GeomipTerrain::AddTriangle(int index, int indexC, int index1, int index2)
{
	indices.push_back(indexC);
	indices.push_back(index1);
	indices.push_back(index2);

	return index + 3;
}

void GeomipTerrain::Render(Shader& shader, Camera& camera)
{
	shader.use();
	lodManager.UpdateLOD(camera.getCameraPos());
	glBindVertexArray(terrainVAO);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	// for wireframe mode
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// setup view projection matrix
	float pw = 1600.0f;
	float ph = 1200.0f;
	glm::mat4 viewProj = camera.getProjectionMatrix(pw, ph, 0.1f, 2500.0f) * camera.getViewMatrix();

	Frustum frustum(viewProj);

	// traverse all patches
	for (int patchZ = 0; patchZ < numPatchesZ; patchZ++)
	{
		for (int patchX = 0; patchX < numPatchesX; patchX++)
		{
			float wx = patchX * (patchSize - 1) * worldScale;
			float wz = patchZ * (patchSize - 1) * worldScale;
			float wPatchSize = (patchSize - 1) * worldScale;
			float patchRad = wPatchSize * glm::sqrt(2.0f) * 0.5f;

			// since height is just based on range 0-1 * heightScale, max height is heightScale
			glm::vec3 patchCenter = glm::vec3(wx + wPatchSize * 0.5f, heightScale * 0.5f, wz + wPatchSize * 0.5f);

			// cull if sphere is outside the frustum
			if (!frustum.IsPatchSphereInFrustum(patchCenter, patchRad))
			{
				// printf("0");
				continue;
			}
			// else printf("1");

			const LODManager::PatchLOD& patchLOD = lodManager.GetPatchLOD(patchX, patchZ);
			// core LOD level
			int c = patchLOD.core;
			// ring LOD levels (0-1, 0 = core, 1 = core + 1)
			int l = patchLOD.left;
			int r = patchLOD.right;
			int t = patchLOD.top;
			int b = patchLOD.bottom;

			// which set of indices to use
			auto& slice = lodInfo[c].info[l][r][t][b];
			size_t baseIndex = sizeof(unsigned int) * slice.start;

			int z = patchZ * (patchSize - 1);
			int x = patchX * (patchSize - 1);
			int baseVertex = z * heightData.width + x;

			glDrawElementsBaseVertex(GL_TRIANGLES, slice.count, GL_UNSIGNED_INT, (void*)baseIndex, baseVertex);
		}
		// printf("\n");
	}
	// printf("\n");
	glBindVertexArray(0);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
}

