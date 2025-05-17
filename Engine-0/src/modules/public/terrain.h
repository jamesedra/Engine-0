#pragma once
#include "../../common.h"
#include "shader.h"

struct HeightData
{
	std::vector<float> data; // height data, ranging from 0 to 1
	int width = 0;
	int depth = 0;

	void unload()
	{
		data.clear();
		width = depth = 0;
	}
};

class Terrain
{
protected:
	HeightData heightData;
	float heightScale = 255.0f;

public:
	Terrain(){}
	~Terrain()
	{
		if (!heightData.data.empty()) UnloadHeightData();
	}

	virtual void Render(Shader& shader) = 0;

	// Height data generation
	bool LoadHeightMap(const char* filename);
	bool GenerateFaultHeightData(int iterations, int width = -1, int height = -1);

	// TODO:
	bool SaveHeightMap(const char* filename);
	bool UnloadHeightData();

	// Normalizes to range 0 - 1
	void NormalizeHeightData();

	inline void SetHeightDataDimensions(unsigned int w = 1000, unsigned int d = 1000)
	{
		heightData.width = w;
		heightData.depth = d;
	}

	inline void SetHeightScale(float scale)
	{
		heightScale = scale;
	}

	// Sets the true height value at the given point
	inline void SetHeightAtPoint(float height, int x, int z)
	{
		heightData.data[(z * heightData.width) + x] = height;
	}

	// Gets the true height at a point
	inline float GetTrueHeightAtPoint(int x, int z)
	{
		return heightData.data[(z * heightData.width) + x];
	}

	// Gets the scaled height at a point
	inline float GetScaledHeightAtPoint(int x, int z)
	{
		return heightData.data[(z * heightData.width) + x] * heightScale;
	}
};

class BruteForceTerrain : public Terrain
{
private:
	// brute force vert struct
	struct BFVertex
	{
		glm::vec3 position, color;
	};

	GLuint terrainVAO, terrainVBO;
	std::vector<BFVertex> verts;

public:
	void Initialize()
	{
		if (heightData.data.empty() || heightData.width == 0 || heightData.depth == 0)
		{
			std::cerr << "Terrain::Initialize called before height data was loaded\n";
			return;
		}

		glGenVertexArrays(1, &terrainVAO);
		glGenBuffers(1, &terrainVBO);

		glBindVertexArray(terrainVAO);
		
		int w = heightData.width;
		int d = heightData.depth;
		verts.reserve(w * 2 * (d - 1));

		// height-based coloring
		for (int z = 0; z < d - 1; z++)
		{
			for (int x = 0; x < w; x++)
			{
				float raw0 = GetTrueHeightAtPoint(x, z);
				float y0 = GetScaledHeightAtPoint(x, z);
				float c0 = raw0;

				verts.push_back({ {float(x), y0,float(z)},{c0,c0,c0} });

				float raw1 = GetTrueHeightAtPoint(x, z + 1);
				float y1 = GetScaledHeightAtPoint(x, z + 1);
				float c1 = raw1;

				verts.push_back({ {float(x), y1, float(z + 1)},  {c1,c1,c1} });
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(BFVertex), verts.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BFVertex), (void*)offsetof(BFVertex, position));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BFVertex), (void*)offsetof(BFVertex, color));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Render(Shader& shader) override
	{
		shader.use();
		glBindVertexArray(terrainVAO);

		int w = heightData.width;
		int d = heightData.depth;
		int vertsPerRow = w * 2;

		for (int z = 0; z < d - 1; ++z) glDrawArrays(GL_TRIANGLE_STRIP, z * vertsPerRow, vertsPerRow);
		glBindVertexArray(0);
	}
};