#pragma once
#include "component_manager.h"

// NOTE: Contexts are structs that holds references of component managers and registries. Mainly uses this since passing multiple parameters are kind of blowing out of proportion.

// World context is used for rendering/viewports
struct WorldContext
{
	EntityManager* entityManager;
	TransformManager* transformManager;
	ShaderManager* shaderManager;
	AssetManager* assetManager;
	MaterialsGroupManager* materialsGroupManager;

	WorldContext(
		EntityManager* entityManager, 
		TransformManager* transformManager, 
		ShaderManager* shaderManager, 
		AssetManager* assetManager,
		MaterialsGroupManager* materialsGroupManager
	): 
		entityManager(entityManager), 
		transformManager(transformManager), 
		shaderManager(shaderManager), 
		assetManager(assetManager),
		materialsGroupManager(materialsGroupManager)
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