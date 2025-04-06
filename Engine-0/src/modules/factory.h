#pragma once
#include "../common.h"
#include "component_manager.h"

// NOTE: this is a tentative header for testing purposes.
// A "Factory" is used to create a combination of components that will be tied from a certain entity. 
// Factories would only help with the creation of it. Adding/removing/editing components that are tied to an entity would be from a different implementation.

class WorldObjectFactory
{
public:
    static Entity CreateWorldMesh(
        EntityManager& entityManager,
        TransformManager& transformManager,
        TestMeshManager& meshManager,
        ShaderManager& shaderManager,
        MaterialManager& materialManager,
        GLuint cubeVAO,
        Shader* gBufferShader)
    {
        Entity entity = entityManager.CreateEntity();


    }
};