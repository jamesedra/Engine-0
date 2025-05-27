#include "../public/terrain_tess.h"

void TessTerrain::InitializePatches()
{
	int width = heightData.width;
	int depth = heightData.depth;

	// guard rails
	if (heightData.data.empty() || width == 0 || depth == 0)
	{
		std::cerr << "TessTerrain::InitializePatches called before height data was properly loaded\n";
		return;
	}

	// patch verts. 
	// Will not use the verts in the parent class since we only need vec2 for pos
	struct PatchData
	{
		glm::vec2 gridPos, uv;
	};
	std::vector<PatchData> pverts;
	pverts.reserve(width * depth);
	float repeat = float(width - 1) * 0.25f;

	for (int z = 0; z < depth; z++)
	{
		for (int x = 0; x < width; x++)
		{
			pverts.push_back({
				{ x,z },
				{ x / float(width - 1) * repeat, z / float(depth - 1) * repeat } });
		}
	}

	// indices
	for (int z = 0; z < depth; z++)
	{
		for (int x = 0; x < width; x++)
		{
			GLuint a = z * width + x;
			GLuint b = z * width + (x + 1);
			GLuint c = (z + 1) * width + x;
			GLuint d = (z + 1) * width + (x + 1);
			indices.insert(indices.end(), { a, b, d, c });
		}
	}

	glGenVertexArrays(1, &terrainVAO);
	glGenBuffers(1, &terrainVBO);
	glGenBuffers(1, &terrainEBO);

	glBindVertexArray(terrainVAO);

	glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
	glBufferData(GL_ARRAY_BUFFER, pverts.size() * sizeof(PatchData), pverts.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(PatchData), (void*)offsetof(PatchData, gridPos));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(PatchData), (void*)offsetof(PatchData, uv));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
