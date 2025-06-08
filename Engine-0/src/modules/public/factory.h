#pragma once
#include <filesystem>
#include "../../common.h"
#include "contexts.h"
#include "component_manager.h"
#include "shader_library.h"
#include "asset_library.h"

#include "terrain_brute.h"
#include "terrain_geomip.h"
#include "terrain_tess.h"

#include <map>

// NOTE: this is a tentative header for testing purposes.
// A "Factory" is used to create a combination of components that will be tied from a certain entity. 
// Factories would only help with the creation of it. Adding/removing/editing components that are tied to an entity would be from a different implementation.

// helper on class creation for terrain child types
static std::unique_ptr<Terrain> CreateTerrain(TerrainType type)
{
    switch (type)
    {
    case TerrainType::Brute:
        return std::make_unique<BruteForceTerrain>();
    case TerrainType::Geomipmap:
        return std::make_unique<GeomipTerrain>();
    case TerrainType::Tessellated:
        return std::make_unique<TessTerrain>();
    default:
        throw std::invalid_argument{ "Unknown TerrainType" };
    }
}

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
        shaderComp.shaderName = !shaderLibName.empty() ? shaderLibName : "PBR Test";
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

    static Entity CreateDirectionalLight(
        EntityManager& entityManager,
        LightManager& lightManager,
        TransformManager& transformManager,
        IDManager& idManager,
        std::string name,
        glm::vec3 direction = glm::vec3(3.5f, -30.0f, 50.0f),
        glm::vec3 color = glm::vec3(1.0f),
        float intensity = 5.0f,
        bool enabled = true
    )
    {
        if (lightManager.directionalLightComponents.size() >= 1)
        {
            std::cout << "There is an existing directional light. Overriding.";
            lightManager.RemoveDirectionalLights();
        }

        Entity entity = entityManager.CreateEntity();

        DirectionalLightComponent lightComp{ color, intensity, enabled };
        TransformComponent transformComp;
        transformComp.rotation = direction;

        idManager.components[entity].ID = name;
        lightManager.directionalLightComponents[entity] = std::move(lightComp);
        transformManager.components[entity] = std::move(transformComp);

        return entity;
    }

    static Entity CreatePointLight(
        EntityManager& entityManager,
        LightManager& lightManager,
        TransformManager& transformManager,
        IDManager& idManager,
        std::string name,
        glm::vec3 position = glm::vec3(0.f),
        glm::vec3 color = glm::vec3(1.0f),
        float intensity = 10.0f,
        float radius = 2.5f,
        bool enabled = true
    )
    {
        Entity entity = entityManager.CreateEntity();

        PointLightComponent lightComp{ color, intensity, radius, enabled };
        TransformComponent transformComp;
        transformComp.position = position;

        idManager.components[entity].ID = name;
        lightManager.pointLightComponents[entity] = std::move(lightComp);
        transformManager.components[entity] = std::move(transformComp);

        return entity;
    }

    static Entity CreateEnvironmentProbe(
        EntityManager& entityManager,
        EnvironmentProbeManager& probeManager,
        IDManager& idManager,
        std::string name,
        IBLSettings& settings,
        glm::vec3 position = glm::vec3(0.0f),
        float radius = 5.0f)
    {
        Entity entity = entityManager.CreateEntity();

        EnvironmentProbeComponent probeComp;
        probeComp.settings = settings;
        probeComp.maps = IBLGenerator::Build(probeComp.settings);
        probeComp.buildProbe = false;
        probeComp.position = position;
        probeComp.radius = radius;

        idManager.components[entity].ID = name;
        probeManager.probeComponents[entity] = std::move(probeComp);

        return entity;
    }

    static Entity CreateSkyProbe(
        EntityManager& entityManager,
        EnvironmentProbeManager& probeManager,
        IDManager& idManager,
        std::string name,
        IBLSettings& settings,
        glm::vec3 position = glm::vec3(0.0f)
    )
    {
        Entity entity = entityManager.CreateEntity();

        EnvironmentProbeComponent probeComp;
        probeComp.settings = settings;
        probeComp.maps = IBLGenerator::Build(probeComp.settings);
        probeComp.buildProbe = false;
        probeComp.position = position;
        probeComp.radius = 0.0f;

        idManager.components[entity].ID = name;
        probeManager.AddSkyProbe(entity, probeComp);

        return entity;
    }


    static Entity CreateLandscape(
        EntityManager& entityManager,
        LandscapeManager& landscapeManager,
        IDManager& idManager,
        std::string name,
        TerrainType terrainType,
        HeightGenParams heightParams,
        float heightScale
    )
    {
        Entity entity = entityManager.CreateEntity();
        auto terrainPtr = CreateTerrain(terrainType);
        
        // initial generation of height data
        if (std::holds_alternative<FaultGenParams>(heightParams))
        {
            // TODO
        }
        else if (std::holds_alternative<MidpointGenParams>(heightParams))
        {
            // TODO
        }
        else if (std::holds_alternative<HeightmapParams>(heightParams))
        {
            auto& params = std::get<HeightmapParams>(heightParams);
            terrainPtr->LoadHeightMap(params.filename.c_str());
        }

        terrainPtr->SetHeightScale(heightScale);
        terrainPtr->Initialize();
        LandscapeComponent landComp{ std::move(terrainPtr) };
        HeightGenComponent genComp{ std::move(heightParams), heightScale, false };

        landscapeManager.landscapeComponents[entity] = std::move(landComp);
        landscapeManager.heightGenComponents[entity] = std::move(genComp);

        idManager.components[entity] = name;

        return entity;
    }
};