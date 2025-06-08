#pragma once
#include <unordered_map>
#include <string>
#include "shader_uniform.h"
#include "shader.h"
#include "model.h"

// when the textures are already in the texture library
// we only care about the type and id
struct RegisteredTextureData
{
    std::string key; // key for texture library
    std::string type;
    unsigned int id;
};

struct Material
{
    // uniform name : uniform value
	std::unordered_map<std::string, UniformValue> uniforms;
    std::vector<RegisteredTextureData> matTextures;
    // std::vector<TextureMetadata> materialTextures; // to be fully deprecated

    Material(
        Shader& shader, 
        std::vector<TextureMetadata> texture_data, 
        bool includeTransforms = false, 
        bool includeTextureUniforms = true)
    {
        uniforms = InitializeMaterialComponent(shader.ID, includeTransforms, includeTextureUniforms);

        // from mesh draw
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int metallicNr = 1;
        unsigned int roughnessNr = 1;
        unsigned int aoNr = 1;

        for (size_t i = 0; i < texture_data.size(); i++)
        {
            auto& td = texture_data[i];
            TextureLibrary::Register(td.path, td.id, td.width, td.height);
            // since this is auto attachment, make path as the key
            RegisteredTextureData mt = { td.path, td.type, td.id };
            matTextures.push_back(mt);

            std::string number;
            if (mt.type == "texture_diffuse") number = std::to_string(diffuseNr);
            else if (mt.type == "texture_specular") number = std::to_string(specularNr);
            else if (mt.type == "texture_normal") number = std::to_string(normalNr);
            else if (mt.type == "texture_roughness") number = std::to_string(roughnessNr);
            else if (mt.type == "texture_metallic") number = std::to_string(metallicNr);
            else if (mt.type == "texture_ao") number = std::to_string(aoNr);
            std::string uniformName = "material." + mt.type + number;

            // override if uniform name exists
            auto it = uniforms.find(uniformName);
            if (it != uniforms.end() && it->second.type == UniformValue::Type::Sampler2D)
            {
                it->second = UniformValue::Sampler2D(mt.key);
            }
        }
    }

    Material(
        Shader& shader,
        std::vector<RegisteredTextureData> texture_data,
        bool includeTransforms = false,
        bool includeTextureUniforms = true)
        :
        matTextures(std::move(texture_data))
    {
        uniforms = InitializeMaterialComponent(shader.ID, includeTransforms, includeTextureUniforms);

        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int metallicNr = 1;
        unsigned int roughnessNr = 1;
        unsigned int aoNr = 1;

        for (size_t i = 0; i < matTextures.size(); i++)
        {
            auto& mt = matTextures[i];

            std::string number;
            if (mt.type == "texture_diffuse") number = std::to_string(diffuseNr);
            else if (mt.type == "texture_specular") number = std::to_string(specularNr);
            else if (mt.type == "texture_normal") number = std::to_string(normalNr);
            else if (mt.type == "texture_roughness") number = std::to_string(roughnessNr);
            else if (mt.type == "texture_metallic") number = std::to_string(metallicNr);
            else if (mt.type == "texture_ao") number = std::to_string(aoNr);
            std::string uniformName = "material." + mt.type + number;

            // override if uniform name exists
            auto it = uniforms.find(uniformName);
            if (it != uniforms.end() && it->second.type == UniformValue::Type::Sampler2D)
            {
                it->second = UniformValue::Sampler2D(mt.key);
            }
        }
    }
	
	void ApplyShaderUniforms(Shader& shader) const
	{
        shader.use();
        int textureUnit = 0;
        for (auto& pair : uniforms)
        {
            std::string name = pair.first;
            UniformValue value = pair.second;

            switch (value.type)
            {
                case UniformValue::Type::Bool:
                    shader.setBool(name, value.boolValue);
                    break;
                case UniformValue::Type::Int:
                    shader.setInt(name, value.intValue);
                    break;
                case UniformValue::Type::Float:
                    shader.setFloat(name, value.floatValue);
                    break;
                case UniformValue::Type::Vec2:
                    shader.setVec2(name, value.vec2Value);
                    break;
                case UniformValue::Type::Vec3:
                    shader.setVec3(name, value.vec3Value);
                    break;
                case UniformValue::Type::Vec4:
                    shader.setVec4(name, value.vec4Value);
                    break;
                case UniformValue::Type::Mat4:
                    shader.setMat4(name, value.mat4Value);
                    break;
                case UniformValue::Type::Sampler2D:
                {
                    shader.setInt(name, textureUnit);
                    glActiveTexture(GL_TEXTURE0 + textureUnit);
                    auto& tex = TextureLibrary::GetTexture(value.texturePath);
                    glBindTexture(GL_TEXTURE_2D, tex.id);
                    textureUnit++;
                    break;
                }
                case UniformValue::Type::SamplerCube:
                {
                    shader.setInt(name, textureUnit);
                    glActiveTexture(GL_TEXTURE0 + textureUnit);
                    auto& tex = TextureLibrary::GetTexture(value.texturePath);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, tex.id);
                    textureUnit++;
                    break;
                }
            }
        }
        glActiveTexture(GL_TEXTURE0);
	}

    // when shaders gets swapped at runtime
    void SetShader(Shader& newShader, bool includeTransforms = false, bool includeTextureUniforms = true)
    {
        // reflect new uniform names
        auto newUniforms = InitializeMaterialComponent(newShader.ID, includeTransforms, includeTextureUniforms);

        // use old uniform values if uniforms have the same name
        for (auto& pair : uniforms)
        {
            std::string name = pair.first;
            UniformValue value = pair.second;
            auto it = newUniforms.find(name);
            if (it != newUniforms.end()) it->second = value;
        }

        uniforms = std::move(newUniforms);

        // override samplers
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int metallicNr = 1;
        unsigned int roughnessNr = 1;
        unsigned int aoNr = 1;

        for (size_t i = 0; i < matTextures.size(); i++)
        {
            auto& mt = matTextures[i];
            // TextureLibrary::Register(mt.path, mt.id, mt.width, mt.height);

            std::string number;
            if (mt.type == "texture_diffuse") number = std::to_string(diffuseNr);
            else if (mt.type == "texture_specular") number = std::to_string(specularNr);
            else if (mt.type == "texture_normal") number = std::to_string(normalNr);
            else if (mt.type == "texture_roughness") number = std::to_string(roughnessNr);
            else if (mt.type == "texture_metallic") number = std::to_string(metallicNr);
            else if (mt.type == "texture_ao") number = std::to_string(aoNr);
            std::string uniformName = "material." + mt.type + number;

            // override if uniform name already exists
            auto it = uniforms.find(uniformName);
            if (it != uniforms.end() && it->second.type == UniformValue::Type::Sampler2D)
            {
                it->second = UniformValue::Sampler2D(mt.key);
            }
        }
    }
};

// helper struct
struct MaterialsGroup
{
    Material material;
    // store index of each MeshData that has the same texture metadata
    std::vector<unsigned int> assetPartsIndices;
};
