#pragma once
#include "mesh.h"

// NOTE: This is are functions for loaders or automated class creators. Mostly for default values and testing.

class MeshLoader
{
public:
	static Mesh CreateCube()
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<MeshTexture> textures;

		float f_vertices[] = {
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

		size_t vertexCount = sizeof(f_vertices) / (8 * sizeof(float));
		vertices.resize(vertexCount);

		for (unsigned int i = 0; i < vertexCount; i++)
		{
			size_t offset = i * 8;
			vertices[i].Position = glm::vec3(f_vertices[offset + 0], f_vertices[offset + 1], f_vertices[offset + 2]);
			vertices[i].Normal = glm::vec3(f_vertices[offset + 3], f_vertices[offset + 4], f_vertices[offset + 5]);
			vertices[i].TexCoords = glm::vec2(f_vertices[offset + 6], f_vertices[offset + 7]);

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

		return Mesh(vertices, indices, textures);
	}

private:
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