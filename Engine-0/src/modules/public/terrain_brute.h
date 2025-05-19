#pragma once
#include "terrain.h"

class BruteForceTerrain : public Terrain
{
public:
	void Initialize()
	{
		if (heightData.data.empty() || heightData.width == 0 || heightData.depth == 0)
		{
			std::cerr << "Terrain::Initialize called before height data was loaded\n";
			return;
		}

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

		// indices
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

		// upload
		glGenVertexArrays(1, &terrainVAO);
		glGenBuffers(1, &terrainVBO);
		glGenBuffers(1, &terrainEBO);

		glBindVertexArray(terrainVAO);

		glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(HeightVertexData), verts.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(HeightVertexData), (void*)offsetof(HeightVertexData, pos));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(HeightVertexData), (void*)offsetof(HeightVertexData, normal));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(HeightVertexData), (void*)offsetof(HeightVertexData, uv));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(HeightVertexData), (void*)offsetof(HeightVertexData, tangent));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Render(Shader& shader) override
	{
		shader.use();
		glBindVertexArray(terrainVAO);
		glDrawElements(GL_TRIANGLES, GLsizei(indices.size()), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}
};