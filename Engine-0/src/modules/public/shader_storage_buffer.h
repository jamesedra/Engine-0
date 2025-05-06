#pragma once

#include <iostream>
#include <cassert>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ShaderStorageBuffer
{
public:
	unsigned int SSBO;

	ShaderStorageBuffer(GLuint bindingPoint, GLsizei bufferSize, GLsizeiptr bufferDataSize, const void* data = nullptr, GLenum usage = GL_DYNAMIC_DRAW)
	{
		glGenBuffers(bufferSize, &SSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, bufferDataSize, data, usage);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, SSBO);
	}

	void bind()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
	}

	void setData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
		glBufferSubData(target, offset, size, data);
	}
};