#include "../public/terrain_geomip.h"

void GeomipTerrain::GenerateGeomip(int patchSize)
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

	// assign maxLOD properly later after an lod manager is built
	this->maxLOD = 3;
	lodInfo.resize(maxLOD + 1);

	Initialize();
}

void GeomipTerrain::Initialize()
{
	int w = heightData.width;
	int d = heightData.depth;

	// normal helper
	auto sampleNormal = [&](int x, int z)
		{
			int xm = std::max(x - 1, 0), xp = std::min(x + 1, w - 1);
			int zm = std::max(z - 1, 0), zp = std::min(z + 1, d - 1);

			float hL = GetScaledHeightAtPoint(xm, z), hR = GetScaledHeightAtPoint(xp, z);
			float hD = GetScaledHeightAtPoint(x, zm), hU = GetScaledHeightAtPoint(x, zp);

			glm::vec3 dx = glm::vec3(1.0f, hR - hL, 0.0f);
			glm::vec3 dz = glm::vec3(0.0f, hU - hD, 1.0f);
			glm::vec3 n = glm::normalize(glm::cross(dz, dx));
			return n;
		};

	// verts
	verts.clear();
	verts.reserve(w * d);

	float repeat = float(w - 1) * 0.25f; // for texture repetition
	for (int z = 0; z < d; z++)
	{
		for (int x = 0; x < w; x++)
		{
			HeightVertexData hvd{};

			hvd.pos = glm::vec3(float(x), GetScaledHeightAtPoint(x, z), float(z));
			hvd.normal = sampleNormal(x, z);
			hvd.uv = glm::vec2(float(x) / (w - 1) * repeat, float(z) / (d - 1) * repeat);
			hvd.tangent = glm::normalize(
				glm::vec3(1, GetScaledHeightAtPoint(x + 1 < w ? x + 1 : x, z) - GetScaledHeightAtPoint(x, z), 0));

			verts.push_back(hvd);
		}
	}
}

void GeomipTerrain::Render(Shader& shader)
{
}
