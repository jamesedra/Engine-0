#pragma once
#include "../../common.h"

struct UniformValue
{
    enum class Type { Bool, Int, Float, Vec2, Vec3, Vec4, Mat4, Sampler2D, SamplerCube } type;
    union
    {
        bool boolValue;
        int intValue;
        float floatValue;
        glm::vec2 vec2Value;
        glm::vec3 vec3Value;
        glm::vec4 vec4Value;
        glm::mat4 mat4Value;
    };

	std::string texturePath; // path of the texture. Used for Texture Libraries

    UniformValue(bool value) : type(Type::Bool), boolValue(value) {}
    UniformValue(int value) : type(Type::Int), intValue(value) {}
    UniformValue(float value) : type(Type::Float), floatValue(value) {}
    UniformValue(glm::vec2 value) : type(Type::Vec2), vec2Value(value) {}
    UniformValue(glm::vec3 value) : type(Type::Vec3), vec3Value(value) {}
    UniformValue(glm::vec4 value) : type(Type::Vec4), vec4Value(value) {}
    UniformValue(glm::mat4 value) : type(Type::Mat4), mat4Value(value) {}
	UniformValue(Type t = Type::Bool) : type(t), intValue(0) {}
	static UniformValue Sampler2D(const std::string&path)
	{
		UniformValue u(Type::Sampler2D);   
		u.texturePath = path;
		return u;
	}
	static UniformValue SamplerCube(const std::string& path)
	{
		UniformValue u(Type::SamplerCube);  
		u.texturePath = path;
		return u;
	}
};

// Commenting out unused uniforms
std::string UniformTypeToString(GLenum type)
{
	switch (type)
	{
		case GL_FLOAT:             return "GL_FLOAT";
		case GL_FLOAT_VEC2:        return "GL_FLOAT_VEC2";
		case GL_FLOAT_VEC3:        return "GL_FLOAT_VEC3";
		case GL_FLOAT_VEC4:        return "GL_FLOAT_VEC4";
		case GL_INT:               return "GL_INT";
		//case GL_INT_VEC2:          return "GL_INT_VEC2";
		//case GL_INT_VEC3:          return "GL_INT_VEC3";
		//case GL_INT_VEC4:          return "GL_INT_VEC4";
		case GL_BOOL:              return "GL_BOOL";
		//case GL_BOOL_VEC2:         return "GL_BOOL_VEC2";
		//case GL_BOOL_VEC3:         return "GL_BOOL_VEC3";
		//case GL_BOOL_VEC4:         return "GL_BOOL_VEC4";
		//case GL_FLOAT_MAT2:        return "GL_FLOAT_MAT2";
		//case GL_FLOAT_MAT3:        return "GL_FLOAT_MAT3";
		case GL_FLOAT_MAT4:        return "GL_FLOAT_MAT4";
		case GL_SAMPLER_2D:        return "GL_SAMPLER_2D";
		case GL_SAMPLER_CUBE:      return "GL_SAMPLER_CUBE";
		default:                   return "UNKNOWN";
	}
}

UniformValue UniformTypeToValue(GLenum type)
{
	switch (type)
	{
		case GL_FLOAT:             return UniformValue(1.0f);
		case GL_FLOAT_VEC2:        return UniformValue(glm::vec2(1.0f));
		case GL_FLOAT_VEC3:        return UniformValue(glm::vec3(1.0f));
		case GL_FLOAT_VEC4:        return UniformValue(glm::vec4(1.0f));
		case GL_INT:               return UniformValue(0);
		//case GL_INT_VEC2:          return "GL_INT_VEC2";
		//case GL_INT_VEC3:          return "GL_INT_VEC3";
		//case GL_INT_VEC4:          return "GL_INT_VEC4";
		case GL_BOOL:              return UniformValue(false);
		//case GL_BOOL_VEC2:         return "GL_BOOL_VEC2";
		//case GL_BOOL_VEC3:         return "GL_BOOL_VEC3";
		//case GL_BOOL_VEC4:         return "GL_BOOL_VEC4";
		//case GL_FLOAT_MAT2:        return "GL_FLOAT_MAT2";
		//case GL_FLOAT_MAT3:        return "GL_FLOAT_MAT3";
		case GL_FLOAT_MAT4:        return UniformValue(glm::mat4(1.0f));
		case GL_SAMPLER_2D:        return UniformValue::Sampler2D("White Texture - Default");
		case GL_SAMPLER_CUBE:      return UniformValue::SamplerCube("White Texture - Default");
		default:                   return UniformValue();
	}
}

// ran when uniform type detected is sampler 2D to auto assign default textures correctly.
UniformValue UniformSampler2DToValue(std::string name)
{
	struct TexAssignments
	{
		std::string keyword;
		UniformValue value;
	};

	std::vector<TexAssignments> assignments = {
		{ "diffuse", UniformValue::Sampler2D("White Texture - Default") },
		{ "normal", UniformValue::Sampler2D("Normal Texture - Default") },
		{ "specular", UniformValue::Sampler2D("White Texture - Default") },
		{ "ao", UniformValue::Sampler2D("Black Texture - Default") },
		{ "roughness", UniformValue::Sampler2D("White Texture - Default") },
		{ "metallic", UniformValue::Sampler2D("Black Texture - Default") },
	};

	for (auto& a : assignments)
		if (name.find(a.keyword) != std::string::npos)
			return a.value;

	// fallback if none
	return UniformValue::Sampler2D("White Texture - Default");
}


bool containsAnyForTransformUniforms(std::string data_name)
{
	std::vector<std::string> substrings = { "model", "view", "projection" };

	return std::any_of(substrings.begin(), substrings.end(), [&](const std::string& s)
		{
			return data_name.find(s) != std::string::npos;
		});
}

bool containsAnyForTextureUniforms(std::string data_name)
{
	std::vector<std::string> substrings = { "texture_diffuse", "texture_normal", "texture_specular" };

	return std::any_of(substrings.begin(), substrings.end(), [&](const std::string& s)
		{
			return data_name.find(s) != std::string::npos;
		});
}

// reflects uniform names and value types from shader ID
std::unordered_map<std::string, UniformValue> InitializeMaterialComponent(unsigned int shaderID, bool includeTransformUniforms = false, bool includeTextureUniforms = false)
{
	std::unordered_map<std::string, UniformValue> uniforms;

	GLint uniformCount;
	glGetProgramiv(shaderID, GL_ACTIVE_UNIFORMS, &uniformCount);
	GLint maxNameLength;
	glGetProgramiv(shaderID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

	for (unsigned int i = 0; i < uniformCount; i++)
	{
		std::string data_name;
		GLint length = 0;
		GLenum type;
		GLint size;
		std::vector<GLchar> nameData(maxNameLength);
		glGetActiveUniform(shaderID, i, maxNameLength, &length, &size, &type, &nameData[0]);

		// Only handles names and types
		data_name = std::string(nameData.data(), length);
		
		if ((includeTransformUniforms || !containsAnyForTransformUniforms(data_name)))
		{
			uniforms[data_name] = type != GL_SAMPLER_2D ? 
				UniformTypeToValue(type) : UniformSampler2DToValue(data_name);
		}
	}
	return uniforms;
}

