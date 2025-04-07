#pragma once
#include "../common.h"
#include "component_manager.h"
#include "mesh_library.h"
#include "shader_library.h"

// NOTE: this is a tentative header for testing purposes.
// A "Factory" is used to create a combination of components that will be tied from a certain entity. 
// Factories would only help with the creation of it. Adding/removing/editing components that are tied to an entity would be from a different implementation.

class WorldObjectFactory
{
public:
    static Entity CreateWorldMesh(
        EntityManager& entityManager,
        TransformManager& transformManager,
        MeshManager& meshManager,
        ShaderManager& shaderManager,
        MaterialManager& materialManager,
        Mesh* mesh = nullptr,
        Shader* shader = nullptr)
    {
        Entity entity = entityManager.CreateEntity();

        MeshComponent meshComp;
        meshComp.mesh = mesh != nullptr ? mesh : &MeshLibrary::GetMesh("Sphere");
        meshManager.components[entity] = meshComp;

        ShaderComponent shaderComp;
        shaderComp.shader = shader != nullptr ? shader : &ShaderLibrary::GetShader("Default");
        shaderManager.components[entity] = shaderComp;

        TransformComponent transformComp;
        transformComp.position = glm::vec3(0.0f, 0.0f, 0.0f);
        transformComp.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        transformComp.scale = glm::vec3(1.0f);
        transformManager.components[entity] = transformComp;

        MaterialComponent materialComp;
        materialComp.parameters = InitializeMaterialComponent(shaderComp.shader->ID);

        materialManager.components[entity] = materialComp;

        return entity;
    }
};