#pragma once
#include "../../common.h"
#include "contexts.h"
#include "mesh_library.h"
#include "shader_library.h"

// NOTE: this is a tentative header for testing purposes.
// A "Factory" is used to create a combination of components that will be tied from a certain entity. 
// Factories would only help with the creation of it. Adding/removing/editing components that are tied to an entity would be from a different implementation.

class WorldObjectFactory
{
public:
    static Entity CreateWorldMesh(
        WorldContext& worldContext, 
        const std::string meshLibName = "",
        const std::string shaderLibName = "")
    {
        Entity entity = worldContext.entityManager->CreateEntity();

        MeshComponent meshComp;
        meshComp.meshName = !meshLibName.empty() ? meshLibName : "Cube";
        meshComp.mesh = &MeshLibrary::GetMesh(meshComp.meshName);
        worldContext.meshManager->components[entity] = meshComp;

        ShaderComponent shaderComp;
        shaderComp.shaderName = !shaderLibName.empty() ? shaderLibName : "Default Lit";
        shaderComp.shader = &ShaderLibrary::GetShader(shaderComp.shaderName);
        worldContext.shaderManager->components[entity] = shaderComp;

        TransformComponent transformComp;
        transformComp.position = glm::vec3(0.0f, 0.0f, 0.0f);
        transformComp.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        transformComp.scale = glm::vec3(1.0f);
        worldContext.transformManager->components[entity] = transformComp;
      
        MaterialComponent materialComp{
            Material{*shaderComp.shader, {}}
        };
        worldContext.materialManager->components.emplace(entity, std::move(materialComp));

        return entity;
    }
};