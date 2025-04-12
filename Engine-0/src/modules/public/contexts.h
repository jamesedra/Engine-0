#pragma once
#include "component_manager.h"

// NOTE: Contexts are structs that holds references of component managers and registries. Mainly uses this since passing multiple parameters are kind of blowing out of proportion.

// World context is used for rendering/viewports
struct WorldContext
{
	EntityManager* entityManager;
	TransformManager* transformManager;
	MeshManager* meshManager;
	ShaderManager* shaderManager;
	MaterialManager* materialManager;

	WorldContext(
		EntityManager* entityManager, 
		TransformManager* transformManager, 
		MeshManager* meshManager, 
		ShaderManager* shaderManager, 
		MaterialManager* materialManager
	): 
		entityManager(entityManager), 
		transformManager(transformManager), 
		meshManager(meshManager), 
		shaderManager(shaderManager), 
		materialManager(materialManager)
	{ }
};

// Outliner context is used for the outliner window
struct OutlinerContext
{
	SceneEntityRegistry* sceneRegistry;
	IDManager* idManager;

	OutlinerContext(
		SceneEntityRegistry* sceneRegistry,
		IDManager* idManager
	):
		sceneRegistry(sceneRegistry),
		idManager(idManager)
	{ }

};