#pragma once
#include <filesystem>
#include "../../common.h"
#include "contexts.h"
#include "shader_library.h"
#include "asset_library.h"
#include <map>

// NOTE: this is a tentative header for testing purposes.
// A "Factory" is used to create a combination of components that will be tied from a certain entity. 
// Factories would only help with the creation of it. Adding/removing/editing components that are tied to an entity would be from a different implementation.

class WorldObjectFactory
{
public:
    static Entity CreateWorldObject(
        WorldContext& worldContext,
        const std::string shaderLibName = "",
        const std::string assetLibName = "", 
        const std::string assetPath = "")
    {
        Entity entity = worldContext.entityManager->CreateEntity();

        ShaderComponent shaderComp;
        shaderComp.shaderName = !shaderLibName.empty() ? shaderLibName : "Default Lit";
        shaderComp.shader = &ShaderLibrary::GetShader(shaderComp.shaderName);
        worldContext.shaderManager->components[entity] = shaderComp;

        TransformComponent transformComp;
        transformComp.position = glm::vec3(0.0f, 0.0f, 0.0f);
        transformComp.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        transformComp.scale = glm::vec3(1.0f);
        worldContext.transformManager->components[entity] = transformComp;

        std::string assetName = assetLibName;
        if (assetName.empty())
        {
            if (assetPath.empty())
                assetName = "Cube"; // create a placeholder cube
            else
            {
                std::filesystem::path p(assetPath);
                assetName = p.stem().string();  // filename without extension
            }
        }

        Asset& asset = AssetLibrary::GetAsset(assetName, assetPath);
        AssetComponent assetComp{assetName};
        worldContext.assetManager->components[entity] = assetComp;

        MaterialsGroupComponent materialsGroupComponent;
        using TexturePaths = std::vector<std::string>;
        std::map<TexturePaths, std::vector<unsigned int>> textureIndexMap;
        for (unsigned int i = 0; i < asset.parts.size(); i++)
        {
            TexturePaths texturePaths;
            for (auto& textureMetaData : asset.parts[i].textures)
            {
                texturePaths.push_back(textureMetaData.path);
            }
            textureIndexMap[texturePaths].push_back(i);
        }

        materialsGroupComponent.materialsGroup.reserve(textureIndexMap.size());
        for (auto& [paths, indices] : textureIndexMap)
        {
            std::vector<TextureMetadata> textures = asset.parts[indices[0]].textures;
            Material material(*shaderComp.shader, textures);
            MaterialsGroup materialsGroup{
                material, std::move(indices)
            };
            materialsGroupComponent.materialsGroup.push_back(materialsGroup);
        }
        worldContext.materialsGroupManager->components[entity] = std::move(materialsGroupComponent);

        return entity;
    }
};