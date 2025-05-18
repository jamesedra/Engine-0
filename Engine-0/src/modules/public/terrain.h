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

	// deprecated
	virtual void RenderGrad(Shader& shader) = 0;
	
	// Height data generation
	bool LoadHeightMap(const char* filename);
	bool GenerateFaultHeightData(int iterations, float filter, int width = -1, int depth = -1);
	bool GenerateMidpointDispHeightData(float roughness, int size = -1);

	// TODO:
	bool SaveHeightMap(const char* filename);
	bool UnloadHeightData();

	inline void SetHeightDataDimensions(unsigned int w = 1024, unsigned int d = 1024)
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

private:
	void ApplyIIRFilter(float filter); // infinite impulse response
	void NormalizeHeightData();
	void DiamondStep(int step, float disp);
	void SquareStep(int step, float disp);
};

class BruteForceTerrain : public Terrain
{
private:
	// deprecated vert struct
	struct HeightColorVertex
	{
		glm::vec3 position, color;
	};

	struct HeightVertexData
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec4 tangent; // w = bitangent sign
	};

	GLuint terrainVAO = 0, terrainVBO = 0, terrainEBO = 0;
	std::vector<HeightColorVertex> bfVerts;

	std::vector<HeightVertexData> verts;
	std::vector<unsigned int> indices;

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
		float repeat = float(w - 1) * 0.25f; // for texture repetition
		verts.clear();
		verts.reserve(w * d);

		for (int z = 0; z < d; z++)
		{
			for (int x = 0; x < w; x++)
			{
				HeightVertexData hvd{};
				hvd.pos = glm::vec3(float(x), GetScaledHeightAtPoint(x, z), float(z));
				hvd.normal = sampleNormal(x, z);

				glm::vec3 tan = glm::normalize(
					glm::vec3(1, GetScaledHeightAtPoint(x + 1 < w ? x + 1 : x, z) - GetScaledHeightAtPoint(x, z), 0));

				float bSign = (glm::dot(glm::cross(hvd.normal, tan), glm::vec3(0, 0, 1)) < 0.0f) ? -1.0f : 1.0f;
				hvd.tangent = glm::vec4(tan, bSign);

				hvd.uv = glm::vec2(float(x) / (w - 1) * repeat, float(z) / (d - 1) * repeat);

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
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(HeightVertexData), (void*)offsetof(HeightVertexData, tangent));

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

	// Deprecated
	void InitializeColor()
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
		bfVerts.reserve(w * 2 * (d - 1));

		// height-based coloring
		for (int z = 0; z < d - 1; z++)
		{
			for (int x = 0; x < w; x++)
			{
				float raw0 = GetTrueHeightAtPoint(x, z);
				float y0 = GetScaledHeightAtPoint(x, z);
				float c0 = raw0;

				bfVerts.push_back({ {float(x), y0,float(z)},{c0,c0,c0} });

				float raw1 = GetTrueHeightAtPoint(x, z + 1);
				float y1 = GetScaledHeightAtPoint(x, z + 1);
				float c1 = raw1;

				bfVerts.push_back({ {float(x), y1, float(z + 1)},  {c1,c1,c1} });
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
		glBufferData(GL_ARRAY_BUFFER, bfVerts.size() * sizeof(HeightColorVertex), bfVerts.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(HeightColorVertex), (void*)offsetof(HeightColorVertex, position));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(HeightColorVertex), (void*)offsetof(HeightColorVertex, color));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	// Deprecated
	void RenderGrad(Shader& shader) override
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