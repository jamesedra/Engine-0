#pragma once
#include "../../common.h"
#include "shader.h"
#include "camera.h"

enum class TerrainType
{
	Brute,
	Geomipmap,
	Tessellated
};

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
public:
	Terrain(){}
	~Terrain()
	{
		if (!heightData.data.empty()) UnloadHeightData();
	}

	virtual void Render(Shader& shader, Camera& camera, glm::mat4& model) = 0;
	virtual void Initialize() = 0;
	
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

	inline void SetHeightScale(float scale) { heightScale = scale; }

	inline void SetWorldScale(float scale) { worldScale = scale; }

	// Sets the true height value at the given point
	inline void SetHeightAtPoint(float height, int x, int z)
	{
		heightData.data[(z * heightData.width) + x] = height;
	}

	// Gets the true height at a point
	inline float GetHeightAtPoint(int x, int z)
	{
		return heightData.data[(z * heightData.width) + x];
	}

	// Gets the scaled height at a point
	inline float GetScaledHeightAtPoint(int x, int z)
	{
		return heightData.data[(z * heightData.width) + x] * heightScale;
	}

	inline float GetHeightScale() { return heightScale; }

	inline float GetWorldScale() { return worldScale; }

protected:
	HeightData heightData;
	float heightScale = 255.0f;
	float worldScale = 1.0f;
	// for buffers
	struct HeightVertexData
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec3 tangent;
	};
	GLuint terrainVAO = 0, terrainVBO = 0, terrainEBO = 0;
	std::vector<HeightVertexData> verts;
	std::vector<unsigned int> indices;

	void InitHeightVertexData();
	void PopulateBufferData();

private:
	void ApplyIIRFilter(float filter); // infinite impulse response
	void NormalizeHeightData();
	void DiamondStep(int step, float disp);
	void SquareStep(int step, float disp);
};