#pragma once
#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

#include "shader.h"
#include "texture.h"

enum class TextureColorSpace {
	Linear,
	sRGB
};

unsigned int loadCubemap(std::vector<std::string> faces);
unsigned int createDefaultTexture();
unsigned int loadTexture(const char* path, bool flipVertically, TextureColorSpace space = TextureColorSpace::Linear);
unsigned int loadHDR(const char* path, bool flipVertically);

// vertex array object references
unsigned int createCubeVAO();
unsigned int createSphereVAO(unsigned int& indicesCount, float radius = 1.0f, unsigned int sectorCount = 16, unsigned int stackCount = 16);
unsigned int createQuadVAO();
unsigned int createFrameVAO();
unsigned int createDebugFrameVAO();

// model matrix helper
glm::mat4 computeModelMatrix(const glm::vec3& position, const glm::vec3& scale, float angleDegrees, const glm::vec3& rotationAxis);

void bindTextures(const std::vector<unsigned int>& textures, GLenum textureTarget = GL_TEXTURE_2D, unsigned int startUnit = GL_TEXTURE0);

glm::mat2x3 getTangentBitangentMatrix(glm::vec3 positions[3], glm::vec2 texCoords[3]);
glm::vec3 getVertexPosition(const float* vertices, int index);
glm::vec2 getUVPosition(const float* vertices, int index);

float lerp(float a, float b, float t);

void displayFramebufferTexture(Shader shader, unsigned int frame, unsigned int textureID);

unsigned int createPlaceholderCubemap();