#pragma once
#include "mesh.h"
#include "texture.h"
#include <random>

// NOTE: This is are functions for loaders or automated class creators. Mostly for default values and testing.

const float PI = 3.1415926f;

struct SSAOData
{
	std::vector<glm::vec3> kernel;
	std::vector<glm::vec3> noise;
};

class MeshLoader
{
public:
	static Mesh CreateCube()
	{
		std::vector<Vertex> vertices;

		float baseVertexData[] = {
			// back face (0,0,-1)
			-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,

			 0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,

			// front face (0,0,1)
			-0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,

			 0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,

			// left face (-1,0,0)
			-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,

			-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

			// right face (1,0,0)
			 0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,

			 0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,

			 // bottom face (0,-1,0)
			 -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
			  0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
			  0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,

			  0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
			 -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
			 -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,

			 // top face (0,1,0)
			 -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
			  0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
			  0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,

			  0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
			 -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
			 -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f
		};

		size_t vertexArraySize = sizeof(baseVertexData) / (sizeof(float));

		setMeshVertices(baseVertexData, vertexArraySize, vertices);
		return Mesh(vertices);
	}

	static Mesh CreateSphere(float radius, unsigned int sectorCount, unsigned int stackCount)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		std::vector<float> baseVertData;

		// vertices (pos, normals, uvs)
		for (unsigned int i = 0; i <= stackCount; i++)
		{
			float stackAngle = PI / 2.0f - i * (PI / (float)stackCount);
			float xy = radius * cosf(stackAngle);
			float z = radius * sinf(stackAngle);

			for (unsigned int j = 0; j <= sectorCount; j++)
			{
				float sectorAngle = j * (2 * PI / (float)sectorCount);

				// positions
				float x = xy * cosf(sectorAngle);
				float y = xy * sinf(sectorAngle);
				baseVertData.push_back(x);
				baseVertData.push_back(y);
				baseVertData.push_back(z);

				// normals
				float lengthInv = 1.0 / radius;
				baseVertData.push_back(x * lengthInv);
				baseVertData.push_back(y * lengthInv);
				baseVertData.push_back(z * lengthInv);

				// uvs
				float s = (float)j / (float)sectorCount;
				float t = (float)i / (float)stackCount;
				baseVertData.push_back(s);
				baseVertData.push_back(t);

			}
		}

		// indices
		for (unsigned int i = 0; i < stackCount; i++)
		{
			for (unsigned int j = 0; j < sectorCount; j++)
			{
				unsigned int first = i * (sectorCount + 1) + j;
				unsigned int second = first + sectorCount + 1;

				if (i != 0)
				{
					indices.push_back(first);
					indices.push_back(second);
					indices.push_back(first + 1);
				}
				if (i != (stackCount - 1))
				{
					indices.push_back(first + 1);
					indices.push_back(second);
					indices.push_back(second + 1);
				}
			}
		}

		setMeshVertices(baseVertData, vertices, indices);
		
		return Mesh(vertices, indices);
	}

	static Mesh CreateCone(float radius, float height, unsigned int sectorCount, unsigned int stackCount)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<float> baseVertData;

		// Lateral Surface
		// For stacks from 0 (base) to stackCount (apex)
		for (unsigned int i = 0; i <= stackCount; i++)
		{
			float t = (float)i / (float)stackCount;        // 0 at base, 1 at apex
			float currentRadius = (1.0f - t) * radius;       // radius linearly decreases to 0 at apex
			float y = t * height;                            // y goes from 0 (base) to height (apex)

			for (unsigned int j = 0; j <= sectorCount; j++)
			{
				float sectorAngle = j * (2 * PI / (float)sectorCount);
				float x = currentRadius * cosf(sectorAngle);
				float z = currentRadius * sinf(sectorAngle);

				// Position
				baseVertData.push_back(x);
				baseVertData.push_back(y);
				baseVertData.push_back(z);

				// Lateral Normal
				float nx = cosf(sectorAngle) * height;
				float ny = radius;
				float nz = sinf(sectorAngle) * height;
				glm::vec3 normal = glm::normalize(glm::vec3(nx, ny, nz));
				baseVertData.push_back(normal.x);
				baseVertData.push_back(normal.y);
				baseVertData.push_back(normal.z);

				// UVs
				float u = (float)j / (float)sectorCount;
				float v = t;
				baseVertData.push_back(u);
				baseVertData.push_back(v);
			}
		}

		// Indices for lateral surface
		// Each quad on the lateral surface is composed of two triangles.
		// There are (stackCount) rows and (sectorCount) columns.
		for (unsigned int i = 0; i < stackCount; i++)
		{
			for (unsigned int j = 0; j < sectorCount; j++)
			{
				unsigned int first = i * (sectorCount + 1) + j;
				unsigned int second = first + sectorCount + 1;

				// First triangle
				indices.push_back(first);
				indices.push_back(second);
				indices.push_back(first + 1);

				// Second triangle
				indices.push_back(first + 1);
				indices.push_back(second);
				indices.push_back(second + 1);
			}
		}

		// Base of the Cone
		// Add a center vertex for the base
		unsigned int baseCenterIndex = (unsigned int)(baseVertData.size() / 8);
		// Center position (0,0,0), normal (0,0,-1), uv (0.5,0.5)
		baseVertData.push_back(0.0f);
		baseVertData.push_back(0.0f);
		baseVertData.push_back(0.0f);
		baseVertData.push_back(0.0f);
		baseVertData.push_back(0.0f);
		baseVertData.push_back(-1.0f);
		baseVertData.push_back(0.5f);
		baseVertData.push_back(0.5f);

		// Base perimeter vertices:
		for (unsigned int j = 0; j <= sectorCount; j++)
		{
			float sectorAngle = j * (2 * PI / (float)sectorCount);
			float x = radius * cosf(sectorAngle);
			float y = 0.0f;
			float z = radius * sinf(sectorAngle);

			// Position
			baseVertData.push_back(x);
			baseVertData.push_back(y);
			baseVertData.push_back(z);

			// Normal for base: (0,0,-1)
			baseVertData.push_back(0.0f);
			baseVertData.push_back(0.0f);
			baseVertData.push_back(-1.0f);

			// UV: Map circle to [0,1], with center at (0.5,0.5)
			float u = (x / radius + 1.0f) * 0.5f;
			float v = (y / radius + 1.0f) * 0.5f;
			baseVertData.push_back(u);
			baseVertData.push_back(v);
		}

		// Indices for base (triangle fan)
		unsigned int baseStartIndex = baseCenterIndex + 1;
		for (unsigned int j = 0; j < sectorCount; j++)
		{
			indices.push_back(baseCenterIndex);
			indices.push_back(baseStartIndex + j);
			indices.push_back(baseStartIndex + j + 1);
		}

		setMeshVertices(baseVertData, vertices, indices);

		return Mesh(vertices, indices);
	}

private:
	static void setMeshVertices(
		float baseVertexData[],
		size_t vertexArraySize,
		std::vector<Vertex>& vertices)
	{
		size_t vertexCount = vertexArraySize / 8;
		vertices.resize(vertexCount);

		for (unsigned int i = 0; i < vertexCount; i++)
		{
			size_t offset = i * 8;
			vertices[i].Position = glm::vec3(baseVertexData[offset + 0], baseVertexData[offset + 1], baseVertexData[offset + 2]);
			vertices[i].Normal = glm::vec3(baseVertexData[offset + 3], baseVertexData[offset + 4], baseVertexData[offset + 5]);
			vertices[i].TexCoords = glm::vec2(baseVertexData[offset + 6], baseVertexData[offset + 7]);

			vertices[i].Tangent = glm::vec3(0.0f);
			vertices[i].Bitangent = glm::vec3(0.0f);
		}

		for (size_t i = 0; i < vertexCount; i += 3)
		{
			glm::vec3 positions[3] = { vertices[i].Position, vertices[i + 1].Position, vertices[i + 2].Position };
			glm::vec2 texCoords[3] = { vertices[i].TexCoords, vertices[i + 1].TexCoords, vertices[i + 2].TexCoords };

			glm::mat2x3 tangentBitangent = getTangentBitangentMatrix(positions, texCoords);
			vertices[i].Tangent = tangentBitangent[0];
			vertices[i + 1].Tangent = tangentBitangent[0];
			vertices[i + 2].Tangent = tangentBitangent[0];

			vertices[i].Bitangent = tangentBitangent[1];
			vertices[i + 1].Bitangent = tangentBitangent[1];
			vertices[i + 2].Bitangent = tangentBitangent[1];
		}
	}

	static void setMeshVertices(
		const std::vector<float>& baseVertexData,
		std::vector<Vertex>& vertices, std::vector<unsigned int> indices)
	{
		size_t vertexCount = baseVertexData.size() / 8;
		vertices.resize(vertexCount);

		for (unsigned int i = 0; i < vertexCount; i++)
		{
			size_t offset = i * 8;
			vertices[i].Position = glm::vec3(baseVertexData[offset + 0], baseVertexData[offset + 1], baseVertexData[offset + 2]);
			vertices[i].Normal = glm::vec3(baseVertexData[offset + 3], baseVertexData[offset + 4], baseVertexData[offset + 5]);
			vertices[i].TexCoords = glm::vec2(baseVertexData[offset + 6], baseVertexData[offset + 7]);

			vertices[i].Tangent = glm::vec3(0.0f);
			vertices[i].Bitangent = glm::vec3(0.0f);
		}

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			unsigned int i0 = indices[i];
			unsigned int i1 = indices[i + 1];
			unsigned int i2 = indices[i + 2];

			glm::vec3 positions[3] = { vertices[i0].Position, vertices[i1].Position, vertices[i2].Position };
			glm::vec2 texCoords[3] = { vertices[i0].TexCoords, vertices[i1].TexCoords, vertices[i2].TexCoords };

			glm::mat2x3 tangentBitangent = getTangentBitangentMatrix(positions, texCoords);

			vertices[i0].Tangent += tangentBitangent[0];
			vertices[i1].Tangent += tangentBitangent[0];
			vertices[i2].Tangent += tangentBitangent[0];

			vertices[i0].Bitangent += tangentBitangent[1];
			vertices[i1].Bitangent += tangentBitangent[1];
			vertices[i2].Bitangent += tangentBitangent[1];
		}

		for (size_t i = 0; i < vertices.size(); i++)
		{
			vertices[i].Tangent = glm::normalize(vertices[i].Tangent);
			vertices[i].Bitangent = glm::normalize(vertices[i].Bitangent);
		}

	}

	static glm::mat2x3 getTangentBitangentMatrix(glm::vec3 positions[3], glm::vec2 texCoords[3])
	{
		glm::vec3 edge1 = positions[1] - positions[0];
		glm::vec3 edge2 = positions[2] - positions[0];
		glm::vec2 deltaUV1 = texCoords[1] - texCoords[0];
		glm::vec2 deltaUV2 = texCoords[2] - texCoords[0];

		glm::vec3 tangent, bitangent;

		float denom = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		float f = (fabs(denom) > 1e-6f) ? 1.0f / denom : 0.0f;

		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		glm::mat2x3 tangentBitangent;
		tangentBitangent[0] = tangent;
		tangentBitangent[1] = bitangent;

		return tangentBitangent;
	}
};

class TextureLoader
{
public:
	static Texture CreateWhiteTexture()
	{
		unsigned char white[4] = { 255, 255, 255, 255 };
		Texture whiteTex(1, 1, GL_RGBA, GL_RGBA, white);
		whiteTex.setTexFilter(GL_NEAREST);
		return whiteTex;
	}

	static Texture CreateBlackTexture()
	{
		unsigned char black[4] = { 0, 0, 0, 255 };
		Texture blackTex(1, 1, GL_RGBA, GL_RGBA, black);
		blackTex.setTexFilter(GL_NEAREST);
		return blackTex;
	}

	static Texture CreateNormalTexture()
	{
		unsigned char norm[3] = { 128, 128, 255 };
		Texture normTex(1, 1, GL_RGB, GL_RGB, norm);
		normTex.setTexFilter(GL_NEAREST);
		return normTex;
	}

	static Texture CreateTextureFromImport(
		const char* path, 
		bool flipVertically = false, 
		TextureColorSpace space = TextureColorSpace::Linear)
	{
		stbi_set_flip_vertically_on_load(flipVertically);

		int width, height, nrChannels;
		unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
		if (!data)
		{
			std::cerr << "Failed to load texture" << std::endl;
			stbi_image_free(data);
			return CreateWhiteTexture();
		}

		GLenum baseFormat = GL_RGB;
		if (nrChannels == 1)
			baseFormat = GL_RED;
		else if (nrChannels == 3)
			baseFormat = GL_RGB;
		else if (nrChannels == 4)
			baseFormat = GL_RGBA;

		GLenum internalFormat = baseFormat;

		if (space == TextureColorSpace::sRGB)
		{
			if (baseFormat == GL_RGB)
				internalFormat = GL_SRGB;
			else if (baseFormat == GL_RGBA)
				internalFormat = GL_SRGB_ALPHA;
		}

		Texture tex(width, height, internalFormat, baseFormat, GL_LINEAR, GL_REPEAT, data);
		tex.genMipMap();

		stbi_image_free(data);

		return tex;
	}
};

class NoiseLoader
{
public:
	static SSAOData CreateSSAONoiseKernel()
	{
		static std::default_random_engine gen{ std::random_device{}() };
		static std::uniform_real_distribution<float> rnd{ 0.0f, 1.0f };

		SSAOData data;
		data.kernel.reserve(64);
		for (unsigned i = 0; i < 64; ++i)
		{
			glm::vec3 sample{ rnd(gen) * 2.0f - 1.0f, rnd(gen) * 2.0f - 1.0f, rnd(gen) };
			sample = glm::normalize(sample) * rnd(gen);
			float scale = float(i) / 64.0f;
			scale = glm::mix(0.1f, 1.0f, scale * scale);
			data.kernel.push_back(sample * scale);
		}

		data.noise.reserve(16);
		for (unsigned i = 0; i < 16; ++i)
		{
			data.noise.push_back({ rnd(gen) * 2.0f - 1.0f, rnd(gen) * 2.0f - 1.0f, 0.0f });
		}
		return data;
	}
};