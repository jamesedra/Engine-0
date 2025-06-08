#include "../public/terrain_tess.h"

void TessTerrain::Initialize()
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
	// float repeat = float(width - 1) * 0.25f;
	float repeat = 1.0f;
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
	for (int z = 0; z < depth - 1; z++)
	{
		for (int x = 0; x < width - 1; x++)
		{
			GLuint a = z * width + x;
			GLuint b = z * width + (x + 1);
			GLuint c = (z + 1) * width + x;
			GLuint d = (z + 1) * width + (x + 1);
			indices.insert(indices.end(), { a, b, c, d });
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

void TessTerrain::Render(Shader& shader, Camera& camera, glm::mat4& model)
{
	shader.use();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	shader.setFloat("tessFactor", 8.0);
	shader.setFloat("heightScale", 50.0f);
	shader.setVec2("terrainScale", glm::vec2(1.0f));

	glBindVertexArray(terrainVAO);
	glDrawElements(GL_PATCHES, static_cast<GLuint>(indices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
