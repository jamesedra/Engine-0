#pragma once
#include "terrain.h"

class BruteForceTerrain : public Terrain
{
public:
	void Initialize() override
	{
		if (heightData.data.empty() || heightData.width == 0 || heightData.depth == 0)
		{
			std::cerr << "Terrain::Initialize called before height data was loaded\n";
			return;
		}

		InitHeightVertexData();

		int w = heightData.width;
		int d = heightData.depth;

		// initialize indices (brute force)
		indices.clear();
		indices.reserve((w - 1) * (d - 1) * 6);
		for (int z = 0; z < d - 1; z++)
		{
			for (int x = 0; x < w - 1; x++)
			{
				unsigned int i0 = z * w + x;
				unsigned int i1 = z * w + (x + 1);
				unsigned int i2 = (z + 1) * w + x;
				unsigned int i3 = (z + 1) * w + (x + 1);

				indices.push_back(i0);
				indices.push_back(i1);
				indices.push_back(i3);

				indices.push_back(i0);
				indices.push_back(i3);
				indices.push_back(i2);
			}
		}
		PopulateBufferData();
	}

	void Render(Shader& shader, Camera& camera) override
	{
		shader.use();
		glBindVertexArray(terrainVAO);
		glDrawElements(GL_TRIANGLES, GLsizei(indices.size()), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}
};